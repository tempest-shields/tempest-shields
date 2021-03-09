#include "SparseSmgRpatlHelper.h"

#include "storm/environment/Environment.h"
#include "storm/environment/solver/MultiplierEnvironment.h"
#include "storm/environment/solver/MinMaxSolverEnvironment.h"
#include "storm/solver/MinMaxLinearEquationSolver.h"
#include "storm/utility/vector.h"

#include "storm/modelchecker/rpatl/helper/internal/GameViHelper.h"
#include "storm/modelchecker/rpatl/helper/internal/BoundedGloballyGameViHelper.h"

namespace storm {
    namespace modelchecker {
        namespace helper {

            template<typename ValueType>
            MDPSparseModelCheckingHelperReturnType<ValueType> SparseSmgRpatlHelper<ValueType>::computeUntilProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& phiStates, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint) {
                // TODO add Kwiatkowska original reference
                // TODO FIX solver min max mess

                auto solverEnv = env;
                solverEnv.solver().minMax().setMethod(storm::solver::MinMaxMethod::ValueIteration, false);

                // Initialize the solution vector.
                std::vector<ValueType> x = std::vector<ValueType>(transitionMatrix.getRowGroupCount() - psiStates.getNumberOfSetBits(), storm::utility::zero<ValueType>());

                // Relevant states are those states which are phiStates and not PsiStates.
                storm::storage::BitVector relevantStates = phiStates & ~psiStates;

                std::vector<ValueType> b = transitionMatrix.getConstrainedRowGroupSumVector(relevantStates, psiStates);

                // Reduce the matrix to relevant states
                storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, relevantStates, relevantStates, false);

                storm::storage::BitVector clippedStatesOfCoalition(relevantStates.getNumberOfSetBits());
                clippedStatesOfCoalition.setClippedStatesOfCoalition(relevantStates, statesOfCoalition);
                clippedStatesOfCoalition.complement();

                storm::modelchecker::helper::internal::GameViHelper<ValueType> viHelper(submatrix, clippedStatesOfCoalition);
                std::unique_ptr<storm::storage::Scheduler<ValueType>> scheduler;
                if (produceScheduler) {
                    viHelper.setProduceScheduler(true);
                }

                viHelper.performValueIteration(env, x, b, goal.direction());
                viHelper.fillResultVector(x, relevantStates, psiStates);

                if (produceScheduler) {
                    scheduler = std::make_unique<storm::storage::Scheduler<ValueType>>(expandScheduler(viHelper.extractScheduler(), psiStates, ~phiStates));
                }
                return MDPSparseModelCheckingHelperReturnType<ValueType>(std::move(x), std::move(scheduler));
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
            MDPSparseModelCheckingHelperReturnType<ValueType> SparseSmgRpatlHelper<ValueType>::computeGloballyProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint) {
                // the idea is to implement the definition of globally as in the formula:
                // G psi = not(F(not psi)) = not(true U (not psi))
                // so the psiStates are flipped, then the true U part is calculated, at the end the result is flipped again
                storm::storage::BitVector notPsiStates = ~psiStates;
                statesOfCoalition.complement();
                auto result = computeUntilProbabilities(env, std::move(goal), transitionMatrix, backwardTransitions, storm::storage::BitVector(transitionMatrix.getRowGroupCount(), true), notPsiStates, qualitative, statesOfCoalition, produceScheduler, hint);
                for (auto& element : result.values) {
                    element = storm::utility::one<ValueType>() - element;
                }
                return result;
            }

            template<typename ValueType>
            MDPSparseModelCheckingHelperReturnType<ValueType> SparseSmgRpatlHelper<ValueType>::computeNextProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint) {
                // create vector x for result, bitvector allStates with a true for each state and a vector b for the probability to get to state psi
                std::vector<ValueType> x = std::vector<ValueType>(transitionMatrix.getRowGroupCount(), storm::utility::zero<ValueType>());
                storm::storage::BitVector allStates = storm::storage::BitVector(transitionMatrix.getRowGroupCount(), true);
                std::vector<ValueType> b = transitionMatrix.getConstrainedRowGroupSumVector(allStates, psiStates);

                statesOfCoalition.complement();

                if (produceScheduler) {
                    STORM_LOG_WARN("Next formula does not expect that produceScheduler is set to true.");
                }

                // create multiplier and execute the calculation for 1 step
                auto multiplier = storm::solver::MultiplierFactory<ValueType>().create(env, transitionMatrix);
                multiplier->multiplyAndReduce(env, goal.direction(), x, &b, x, nullptr, &statesOfCoalition);

                STORM_LOG_DEBUG("x = " << storm::utility::vector::toString(x));
                return MDPSparseModelCheckingHelperReturnType<ValueType>(std::move(x));
            }

            template<typename ValueType>
            MDPSparseModelCheckingHelperReturnType<ValueType> SparseSmgRpatlHelper<ValueType>::computeBoundedGloballyProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint,uint64_t lowerBound, uint64_t upperBound) {
                auto solverEnv = env;
                solverEnv.solver().minMax().setMethod(storm::solver::MinMaxMethod::ValueIteration, false);

                // Relevant states are safe states - the psiStates.
                storm::storage::BitVector relevantStates = psiStates;

                // Initialize the solution vector.
                //std::vector<ValueType> x = std::vector<ValueType>(transitionMatrix.getRowGroupCount() - psiStates.getNumberOfSetBits(), storm::utility::zero<ValueType>());
                std::vector<ValueType> x = std::vector<ValueType>(relevantStates.getNumberOfSetBits(), storm::utility::one<ValueType>());

                //std::vector<ValueType> b = transitionMatrix.getConstrainedRowGroupSumVector(relevantStates, relevantStates);
                std::vector<ValueType> b = std::vector<ValueType>(relevantStates.getNumberOfSetBits(), storm::utility::zero<ValueType>());
                // Reduce the matrix to relevant states
                storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, relevantStates, relevantStates, false);

                storm::storage::BitVector clippedStatesOfCoalition(relevantStates.getNumberOfSetBits());
                clippedStatesOfCoalition.setClippedStatesOfCoalition(relevantStates, statesOfCoalition);
                clippedStatesOfCoalition.complement();

                // Use the bounded globally game vi helper
                storm::modelchecker::helper::internal::BoundedGloballyGameViHelper<ValueType> viHelper(submatrix, clippedStatesOfCoalition);
                std::unique_ptr<storm::storage::Scheduler<ValueType>> scheduler;
                if (produceScheduler) {
                    viHelper.setProduceScheduler(true);
                }

                // TODO: the lower bounds are not used
                if(lowerBound != 0)
                {
                    STORM_LOG_WARN("The use of lower bounds is not implemented for bounded globally formulas.");
                }

                STORM_LOG_DEBUG("upperBound = " << upperBound);

                // in case of 'G<1' or 'G<=0' the states with are initially 'safe' are filled with ones
                if(upperBound > 0)
                {
/*                    x = std::vector<ValueType>(relevantStates.size(), storm::utility::one<ValueType>());
                } else {*/
                    STORM_LOG_DEBUG("b = " << b);
                    STORM_LOG_DEBUG("x = " << x);
                    viHelper.performValueIteration(env, x, b, goal.direction(), upperBound);
                }

                viHelper.fillResultVector(x, relevantStates);

                // TODO: i am not sure about that ~psiStates are correct - but I think (~psiStates are unsafe like the ~phiStates before)
                //  maybe I should create another method for this case
                if (produceScheduler) {
                    scheduler = std::make_unique<storm::storage::Scheduler<ValueType>>(expandScheduler(viHelper.extractScheduler(), relevantStates, ~relevantStates));
                }

                STORM_LOG_DEBUG("x = " << x);

                return MDPSparseModelCheckingHelperReturnType<ValueType>(std::move(x), std::move(scheduler));
            }

            template class SparseSmgRpatlHelper<double>;
#ifdef STORM_HAVE_CARL
            template class SparseSmgRpatlHelper<storm::RationalNumber>;
#endif
        }
    }
}
