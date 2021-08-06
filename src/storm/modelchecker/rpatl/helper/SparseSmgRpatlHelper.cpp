#include <utility/graph.h>
#include "SparseSmgRpatlHelper.h"

#include "storm/environment/Environment.h"
#include "storm/environment/solver/MultiplierEnvironment.h"
#include "storm/environment/solver/MinMaxSolverEnvironment.h"
#include "storm/solver/MinMaxLinearEquationSolver.h"
#include "storm/utility/vector.h"
#include "storm/utility/graph.h"
#include "storm/modelchecker/rpatl/helper/internal/GameViHelper.h"

namespace storm {
    namespace modelchecker {
        namespace helper {

            template<typename ValueType>
            SMGSparseModelCheckingHelperReturnType<ValueType> SparseSmgRpatlHelper<ValueType>::computeUntilProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& phiStates, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint) {
                // TODO add Kwiatkowska original reference
                auto solverEnv = env;
                solverEnv.solver().minMax().setMethod(storm::solver::MinMaxMethod::ValueIteration, false);

                // Relevant states are those states which are phiStates and not PsiStates.
                storm::storage::BitVector relevantStates = phiStates & ~psiStates;

                // TODO: check if relevantStates should be used for the size of x
                // Initialize the x vector and solution vector result.
                //std::vector<ValueType> x = std::vector<ValueType>(transitionMatrix.getRowGroupCount() - psiStates.getNumberOfSetBits(), storm::utility::zero<ValueType>());
                std::vector<ValueType> x = std::vector<ValueType>(relevantStates.getNumberOfSetBits(), storm::utility::zero<ValueType>());

                std::vector<ValueType> result = std::vector<ValueType>(transitionMatrix.getRowGroupCount(), storm::utility::zero<ValueType>());

                std::vector<ValueType> b = transitionMatrix.getConstrainedRowGroupSumVector(relevantStates, psiStates);
                std::vector<ValueType> constrainedChoiceValues = std::vector<ValueType>(b.size(), storm::utility::zero<ValueType>());
                std::unique_ptr<storm::storage::Scheduler<ValueType>> scheduler;

                STORM_LOG_DEBUG("statesOfCoalition: " << statesOfCoalition);

                storm::storage::BitVector clippedStatesOfCoalition(relevantStates.getNumberOfSetBits());
                clippedStatesOfCoalition.setClippedStatesOfCoalition(relevantStates, statesOfCoalition);

                STORM_LOG_DEBUG("clippedStatesOfCoalition" << clippedStatesOfCoalition);

                //clippedStatesOfCoalition.complement();
                //STORM_LOG_DEBUG("clippedStatesOfCoalition" << clippedStatesOfCoalition);

                if(!relevantStates.empty()) {
                    // Reduce the matrix to relevant states.
                    storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, relevantStates, relevantStates, false);
                    // Create GameViHelper for computations.
                    storm::modelchecker::helper::internal::GameViHelper<ValueType> viHelper(submatrix, clippedStatesOfCoalition);
                    if (produceScheduler) {
                        viHelper.setProduceScheduler(true);
                    }
                    viHelper.performValueIteration(env, x, b, goal.direction());
                    if(goal.isShieldingTask()) {
                        viHelper.getChoiceValues(env, x, constrainedChoiceValues);
                    }

                    // Fill up the constrainedChoice Values to full size.
                    viHelper.fillChoiceValuesVector(constrainedChoiceValues, relevantStates, transitionMatrix.getRowGroupIndices());

                    if (produceScheduler) {
                        scheduler = std::make_unique<storm::storage::Scheduler<ValueType>>(expandScheduler(viHelper.extractScheduler(), psiStates, ~phiStates));
                    }
                }

                // Fill up the result vector with the values of x for the relevant states, with 1s for psi states (0 is default)
                storm::utility::vector::setVectorValues(result, relevantStates, x);
                storm::utility::vector::setVectorValues(result, psiStates, storm::utility::one<ValueType>());
                STORM_LOG_DEBUG(result);
                return SMGSparseModelCheckingHelperReturnType<ValueType>(std::move(result), std::move(relevantStates), std::move(scheduler), std::move(constrainedChoiceValues));
            }

            template<typename ValueType>
            storm::storage::Scheduler<ValueType> SparseSmgRpatlHelper<ValueType>::expandScheduler(storm::storage::Scheduler<ValueType> scheduler, storm::storage::BitVector psiStates, storm::storage::BitVector notPhiStates) {
                storm::storage::Scheduler<ValueType> completeScheduler(psiStates.size());
                uint_fast64_t maybeStatesCounter = 0;
                uint schedulerSize = psiStates.size();
                for(uint stateCounter = 0; stateCounter < schedulerSize; stateCounter++) {
                    // psiStates already fulfill formulae so we can set an arbitrary action
                    if(psiStates.get(stateCounter)) {
                        completeScheduler.setChoice(0, stateCounter);
                    // ~phiStates do not fulfill formulae so we can set an arbitrary action
                    } else if(notPhiStates.get(stateCounter)) {
                        completeScheduler.setChoice(0, stateCounter);
                    } else {
                        completeScheduler.setChoice(scheduler.getChoice(maybeStatesCounter), stateCounter);
                        maybeStatesCounter++;
                    }
                }
                return completeScheduler;
            }

            template<typename ValueType>
            SMGSparseModelCheckingHelperReturnType<ValueType> SparseSmgRpatlHelper<ValueType>::computeGloballyProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint) {
                // G psi = not(F(not psi)) = not(true U (not psi))
                // The psiStates are flipped, then the true U part is calculated, at the end the result is flipped again.
                storm::storage::BitVector notPsiStates = ~psiStates;
                statesOfCoalition.complement();
                STORM_LOG_DEBUG(statesOfCoalition);

                auto result = computeUntilProbabilities(env, std::move(goal), transitionMatrix, backwardTransitions, storm::storage::BitVector(transitionMatrix.getRowGroupCount(), true), notPsiStates, qualitative, statesOfCoalition, produceScheduler, hint);
                for (auto& element : result.values) {
                    element = storm::utility::one<ValueType>() - element;
                }
                for (auto& element : result.choiceValues) {
                    element = storm::utility::one<ValueType>() - element;
                }
                STORM_LOG_DEBUG(result.values);
                return result;
            }

            template<typename ValueType>
            SMGSparseModelCheckingHelperReturnType<ValueType> SparseSmgRpatlHelper<ValueType>::computeNextProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint) {
                // Create vector result, bitvector allStates with a true for each state and a vector b for the probability for each state to get to a psi state, choiceValues is to store choices for shielding.
                std::vector<ValueType> result = std::vector<ValueType>(transitionMatrix.getRowGroupCount(), storm::utility::zero<ValueType>());
                storm::storage::BitVector allStates = storm::storage::BitVector(transitionMatrix.getRowGroupCount(), true);
                std::vector<ValueType> b = transitionMatrix.getConstrainedRowGroupSumVector(allStates, psiStates);
                std::vector<ValueType> choiceValues = std::vector<ValueType>(transitionMatrix.getRowCount(), storm::utility::zero<ValueType>());
                statesOfCoalition.complement();
                STORM_LOG_DEBUG(statesOfCoalition);

                if (produceScheduler) {
                    STORM_LOG_WARN("Next formula does not expect that produceScheduler is set to true.");
                }
                // Create a multiplier for reduction.
                auto multiplier = storm::solver::MultiplierFactory<ValueType>().create(env, transitionMatrix);
                multiplier->reduce(env, goal.direction(), b, transitionMatrix.getRowGroupIndices(), result, &statesOfCoalition);
                if (goal.isShieldingTask()) {
                    choiceValues = b;
                }
                STORM_LOG_DEBUG(result);
                return SMGSparseModelCheckingHelperReturnType<ValueType>(std::move(result), std::move(allStates), nullptr, std::move(choiceValues));
            }

            template<typename ValueType>
            SMGSparseModelCheckingHelperReturnType<ValueType> SparseSmgRpatlHelper<ValueType>::computeBoundedGloballyProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint,uint64_t lowerBound, uint64_t upperBound) {
                auto solverEnv = env;
                solverEnv.solver().minMax().setMethod(storm::solver::MinMaxMethod::ValueIteration, false);

                // Relevant states are safe states - the psiStates.
                storm::storage::BitVector relevantStates = psiStates;

                // Initializations.
                std::vector<ValueType> x = std::vector<ValueType>(relevantStates.getNumberOfSetBits(), storm::utility::one<ValueType>());
                std::vector<ValueType> b = std::vector<ValueType>(transitionMatrix.getConstrainedRowGroupSumVector(relevantStates, psiStates).size(), storm::utility::zero<ValueType>());
                std::vector<ValueType> result = std::vector<ValueType>(transitionMatrix.getRowGroupCount(), storm::utility::zero<ValueType>());
                std::vector<ValueType> constrainedChoiceValues = std::vector<ValueType>(transitionMatrix.getConstrainedRowGroupSumVector(relevantStates, psiStates).size(), storm::utility::zero<ValueType>());
                std::unique_ptr<storm::storage::Scheduler<ValueType>> scheduler;

                storm::storage::BitVector clippedStatesOfCoalition(relevantStates.getNumberOfSetBits());
                clippedStatesOfCoalition.setClippedStatesOfCoalition(relevantStates, statesOfCoalition);
                //clippedStatesOfCoalition.complement();
                STORM_LOG_DEBUG(clippedStatesOfCoalition);

                if(!relevantStates.empty() && upperBound > 0) {
                    // Reduce the matrix to relevant states.
                    storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, relevantStates, relevantStates, false);
                    // Create GameViHelper for computations.
                    storm::modelchecker::helper::internal::GameViHelper<ValueType> viHelper(submatrix, clippedStatesOfCoalition);
                    if (produceScheduler) {
                        viHelper.setProduceScheduler(true);
                    }

                    viHelper.performValueIterationUpperBound(env, x, b, goal.direction(), upperBound, constrainedChoiceValues);
                    viHelper.fillChoiceValuesVector(constrainedChoiceValues, relevantStates, transitionMatrix.getRowGroupIndices());
                    if (produceScheduler) {
                        scheduler = std::make_unique<storm::storage::Scheduler<ValueType>>(expandScheduler(viHelper.extractScheduler(), relevantStates, ~relevantStates));
                    }
                }

                // Fill up the result vector with the values of x for the relevant states (0 is default)
                storm::utility::vector::setVectorValues(result, relevantStates, x);
                STORM_LOG_DEBUG(result);
                return SMGSparseModelCheckingHelperReturnType<ValueType>(std::move(result), std::move(relevantStates), std::move(scheduler), std::move(constrainedChoiceValues));
            }

            template<typename ValueType>
            SMGSparseModelCheckingHelperReturnType<ValueType> SparseSmgRpatlHelper<ValueType>::computeBoundedUntilProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& phiStates, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint,uint64_t lowerBound, uint64_t upperBound) {
                STORM_LOG_DEBUG("lowerBound = " << lowerBound);
                STORM_LOG_DEBUG("upperBound = " << upperBound);

                auto solverEnv = env;
                solverEnv.solver().minMax().setMethod(storm::solver::MinMaxMethod::ValueIteration, false);

                // relevantStates are default all set
                // if the upper bound is 0 a psiState cannot be passed between 0 and the lower bound - we can reduce the relevant states and fill the psi states in the result with 1s.
                //storm::storage::BitVector relevantStates(transitionMatrix.getRowGroupCount(), true);
                storm::storage::BitVector relevantStates = phiStates & ~psiStates;
                storm::storage::BitVector makeZeroColumns;
/*                if (goal.minimize()) {
                    relevantStates = storm::utility::graph::performProbGreater0A(transitionMatrix, transitionMatrix.getRowGroupIndices(), backwardTransitions, phiStates, psiStates, true, upperBound);
                } else {
                    relevantStates = storm::utility::graph::performProbGreater0E(backwardTransitions, phiStates, psiStates, true, upperBound);
                }*/
/*                if(lowerBound == 0) {
                    STORM_LOG_DEBUG("LOWER BOUND = 0, relevant states are the not-psiStates");
                    relevantStates = phiStates & ~psiStates;
                }*/ /*else {
                    STORM_LOG_DEBUG("LOWER BOUND !=0, relevant states are all states, makeZeroColumns = psiStates");
                    makeZeroColumns = psiStates;
                }*/

                STORM_LOG_DEBUG("relevantStates = " << relevantStates);

                // Initializations.
                std::vector<ValueType> x = std::vector<ValueType>(relevantStates.getNumberOfSetBits(), storm::utility::zero<ValueType>());
                std::vector<ValueType> b = transitionMatrix.getConstrainedRowGroupSumVector(relevantStates, psiStates);
                std::vector<ValueType> result = std::vector<ValueType>(transitionMatrix.getRowGroupCount(), storm::utility::zero<ValueType>());
                std::vector<ValueType> constrainedChoiceValues = std::vector<ValueType>(transitionMatrix.getConstrainedRowGroupSumVector(relevantStates, psiStates).size(), storm::utility::zero<ValueType>());
                std::unique_ptr<storm::storage::Scheduler<ValueType>> scheduler;

                storm::storage::BitVector clippedStatesOfCoalition(relevantStates.getNumberOfSetBits());
                clippedStatesOfCoalition.setClippedStatesOfCoalition(relevantStates, statesOfCoalition);
                //clippedStatesOfCoalition.complement();
                STORM_LOG_DEBUG(clippedStatesOfCoalition);

                if(!relevantStates.empty() && upperBound > 0) {
                    // Reduce the matrix to relevant states. - relevant states are all states
                    storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, relevantStates, relevantStates, false, makeZeroColumns);
                    // Create GameViHelper for computations.
                    storm::modelchecker::helper::internal::GameViHelper<ValueType> viHelper(submatrix, clippedStatesOfCoalition);
                    if (produceScheduler) {
                        viHelper.setProduceScheduler(true);
                    }
                    if(lowerBound == 0) {
                        STORM_LOG_DEBUG("LOWER BOUND = 0 ...");
                        STORM_LOG_DEBUG("Just peform Value Iteration with an UPPER BOUND.");
                        viHelper.performValueIterationUpperBound(env, x, b, goal.direction(), upperBound, constrainedChoiceValues);
                    } else {
                        STORM_LOG_DEBUG("LOWER BOUND != 0 ...");

                        // TODO: change the computation: first the diff, then till the lowerBound

                        STORM_LOG_DEBUG("first multiplication ...");
                        viHelper.performValueIterationUpperBound(env, x, b, goal.direction(), upperBound - lowerBound + 1, constrainedChoiceValues);
                        STORM_LOG_DEBUG("x = " << x);
                        STORM_LOG_DEBUG("b = " << b);
                        STORM_LOG_DEBUG("constrainedChoiceValues = " << constrainedChoiceValues);

                        STORM_LOG_DEBUG("Reduction for ~phiStates ...");
 /*                       for(uint rowGroupIndex = 0; rowGroupIndex < submatrix.getRowGroupCount(); rowGroupIndex++) {
                            for(uint rowIndex = 0; rowIndex < submatrix.getRowGroupSize(rowGroupIndex); rowIndex++) {
                                if(!phiStates.get(rowGroupIndex)) {
                                    constrainedChoiceValues.at(rowGroupIndex) = storm::utility::zero<ValueType>();
                                }
                            }
                        }*/
                        STORM_LOG_DEBUG("constrainedChoiceValues = " << constrainedChoiceValues);

                        //storm::storage::BitVector newRelevantStates(x.size(), false);
                        //storm::utility::vector::setNonzeroIndices(x, newRelevantStates);
                        //storm::utility::vector::setVectorValues(x, ~phiStates, storm::utility::zero<ValueType>());

                        STORM_LOG_DEBUG("reset x ...");
                        x = std::vector<ValueType>(x.size(), storm::utility::zero<ValueType>());
                        STORM_LOG_DEBUG("x = " << x);

                        STORM_LOG_DEBUG("second multiplication ...");
                        viHelper.performValueIterationUpperBound(env, x, constrainedChoiceValues, goal.direction(), lowerBound, constrainedChoiceValues);
                        STORM_LOG_DEBUG("x = " << x);
                        STORM_LOG_DEBUG("b = " << b);
                        STORM_LOG_DEBUG("constrainedChoiceValues = " << constrainedChoiceValues);


/*
                        viHelper.performValueIterationUpperBound(env, x, b, goal.direction(), upperBound - lowerBound + 1, constrainedChoiceValues);
                        submatrix = transitionMatrix.getSubmatrix(true, relevantStates, relevantStates, false);
                        viHelper.updateTransitionMatrix(submatrix);
                        b = std::vector<ValueType>(b.size(), storm::utility::zero<ValueType>());
                        viHelper.performValueIterationUpperBound(env, x, b, goal.direction(), lowerBound - 1, constrainedChoiceValues);
*/

                    }
                    viHelper.fillChoiceValuesVector(constrainedChoiceValues, relevantStates, transitionMatrix.getRowGroupIndices());
                    if (produceScheduler) {
                        scheduler = std::make_unique<storm::storage::Scheduler<ValueType>>(expandScheduler(viHelper.extractScheduler(), relevantStates, ~relevantStates));
                    }
                    storm::utility::vector::setVectorValues(result, relevantStates, x);
                }
                if(lowerBound == 0) {
                    storm::utility::vector::setVectorValues(result, psiStates, storm::utility::one<ValueType>());
                }
                STORM_LOG_DEBUG(result);
                return SMGSparseModelCheckingHelperReturnType<ValueType>(std::move(result), std::move(relevantStates), std::move(scheduler), std::move(constrainedChoiceValues));
            }

            template class SparseSmgRpatlHelper<double>;
#ifdef STORM_HAVE_CARL
            template class SparseSmgRpatlHelper<storm::RationalNumber>;
#endif
        }
    }
}
