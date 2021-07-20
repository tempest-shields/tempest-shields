#include "storm/modelchecker/helper/finitehorizon/SparseNondeterministicStepBoundedHorizonHelper.h"
#include "storm/modelchecker/hints/ExplicitModelCheckerHint.h"
#include "storm/modelchecker/prctl/helper/SparseMdpEndComponentInformation.h"

#include "storm/models/sparse/StandardRewardModel.h"

#include "storm/utility/macros.h"
#include "storm/utility/vector.h"
#include "storm/utility/graph.h"

#include "storm/storage/expressions/Expression.h"
#include "storm/solver/Multiplier.h"
#include "storm/utility/SignalHandler.h"
#include "storm/environment/solver/MinMaxSolverEnvironment.h"

namespace storm {
    namespace modelchecker {
        namespace helper {

            template<typename ValueType>
            SparseNondeterministicStepBoundedHorizonHelper<ValueType>::SparseNondeterministicStepBoundedHorizonHelper(/*storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions*/)
            //transitionMatrix(transitionMatrix), backwardTransitions(backwardTransitions)
            {
                // Intentionally left empty.
            }

            template<typename ValueType>
            std::vector<ValueType> SparseNondeterministicStepBoundedHorizonHelper<ValueType>::compute(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& phiStates, storm::storage::BitVector const& psiStates, uint64_t lowerBound, uint64_t upperBound, ModelCheckerHint const& hint, storm::storage::BitVector& resultMaybeStates, std::vector<ValueType>& choiceValues)
            {
                std::vector<ValueType> result(transitionMatrix.getRowGroupCount(), storm::utility::zero<ValueType>());
                storm::storage::BitVector makeZeroColumns;


                // Determine the states that have 0 probability of reaching the target states.
                storm::storage::BitVector maybeStates;
                if (hint.isExplicitModelCheckerHint() && hint.template asExplicitModelCheckerHint<ValueType>().getComputeOnlyMaybeStates()) {
                    maybeStates = hint.template asExplicitModelCheckerHint<ValueType>().getMaybeStates();
                } else {
                    if (goal.minimize()) {
                        maybeStates = storm::utility::graph::performProbGreater0A(transitionMatrix, transitionMatrix.getRowGroupIndices(), backwardTransitions, phiStates, psiStates, true, upperBound);
                    } else {
                        maybeStates = storm::utility::graph::performProbGreater0E(backwardTransitions, phiStates, psiStates, true, upperBound);
                    }
                    if (lowerBound == 0) {
                        maybeStates &= ~psiStates;
                    } else {
                        makeZeroColumns = psiStates;
                    }

                }

                STORM_LOG_INFO("Preprocessing: " << maybeStates.getNumberOfSetBits() << " non-target states with probability greater 0.");

                if (!maybeStates.empty()) {
                    // We can eliminate the rows and columns from the original transition probability matrix that have probability 0.
                    storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, maybeStates, maybeStates, false, makeZeroColumns);
                    std::vector<ValueType> b = transitionMatrix.getConstrainedRowGroupSumVector(maybeStates, psiStates);

                    // Create the vector with which to multiply.
                    std::vector<ValueType> subresult(maybeStates.getNumberOfSetBits());

                    auto multiplier = storm::solver::MultiplierFactory<ValueType>().create(env, submatrix);
                    if (lowerBound == 0) {
                        if(goal.isShieldingTask())
                        {

                            std::vector<storm::storage::SparseMatrix<double>::index_type> rowGroupIndices = transitionMatrix.getRowGroupIndices();
                            std::vector<storm::storage::SparseMatrix<double>::index_type> reducedRowGroupIndices;
                            uint sizeChoiceValues = 0;
                            for(uint counter = 0; counter < maybeStates.size(); counter++) {
                                if(maybeStates.get(counter)) {
                                    sizeChoiceValues += transitionMatrix.getRowGroupSize(counter);
                                    reducedRowGroupIndices.push_back(rowGroupIndices.at(counter));
                                }
                            }
                            choiceValues = std::vector<ValueType>(sizeChoiceValues, storm::utility::zero<ValueType>());

                            multiplier->repeatedMultiplyAndReduceWithChoices(env, goal.direction(), subresult, &b, upperBound, nullptr, choiceValues, reducedRowGroupIndices);

                            // fill up choicesValues for shields
                            std::vector<ValueType> allChoices = std::vector<ValueType>(transitionMatrix.getRowGroupIndices().at(transitionMatrix.getRowGroupIndices().size() - 1), storm::utility::zero<ValueType>());
                            auto choice_it = choiceValues.begin();
                            for(uint state = 0; state < transitionMatrix.getRowGroupIndices().size() - 1; state++) {
                                uint rowGroupSize = transitionMatrix.getRowGroupIndices().at(state + 1) - transitionMatrix.getRowGroupIndices().at(state);
                                if (maybeStates.get(state)) {
                                    for(uint choice = 0; choice < rowGroupSize; choice++, choice_it++) {
                                        allChoices.at(transitionMatrix.getRowGroupIndices().at(state) + choice) = *choice_it;
                                    }
                                }
                            }
                            choiceValues = allChoices;
                        } else {
                            multiplier->repeatedMultiplyAndReduce(env, goal.direction(), subresult, &b, upperBound);
                        }
                    } else {
                        if(goal.isShieldingTask())
                        {
                            std::vector<storm::storage::SparseMatrix<double>::index_type> rowGroupIndices = transitionMatrix.getRowGroupIndices();
                            std::vector<storm::storage::SparseMatrix<double>::index_type> reducedRowGroupIndices;
                            uint sizeChoiceValues = 0;
                            for(uint counter = 0; counter < maybeStates.size(); counter++) {
                                if(maybeStates.get(counter)) {
                                    sizeChoiceValues += transitionMatrix.getRowGroupSize(counter);
                                    reducedRowGroupIndices.push_back(rowGroupIndices.at(counter));
                                }
                            }
                            choiceValues = std::vector<ValueType>(sizeChoiceValues, storm::utility::zero<ValueType>());

                            multiplier->repeatedMultiplyAndReduceWithChoices(env, goal.direction(), subresult, &b, upperBound - lowerBound + 1, nullptr, choiceValues, reducedRowGroupIndices);
                            storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, maybeStates, maybeStates, false);
                            auto multiplier = storm::solver::MultiplierFactory<ValueType>().create(env, submatrix);
                            b = std::vector<ValueType>(b.size(), storm::utility::zero<ValueType>());
                            multiplier->repeatedMultiplyAndReduceWithChoices(env, goal.direction(), subresult, &b, lowerBound - 1, nullptr, choiceValues, reducedRowGroupIndices);

                            // fill up choicesValues for shields
                            std::vector<ValueType> allChoices = std::vector<ValueType>(transitionMatrix.getRowGroupIndices().at(transitionMatrix.getRowGroupIndices().size() - 1), storm::utility::zero<ValueType>());
                            auto choice_it = choiceValues.begin();
                            for(uint state = 0; state < transitionMatrix.getRowGroupIndices().size() - 1; state++) {
                                uint rowGroupSize = transitionMatrix.getRowGroupIndices().at(state + 1) - transitionMatrix.getRowGroupIndices().at(state);
                                if (maybeStates.get(state)) {
                                    for(uint choice = 0; choice < rowGroupSize; choice++, choice_it++) {
                                        allChoices.at(transitionMatrix.getRowGroupIndices().at(state) + choice) = *choice_it;
                                    }
                                }
                            }
                            choiceValues = allChoices;
                        } else {
                            multiplier->repeatedMultiplyAndReduce(env, goal.direction(), subresult, &b, upperBound - lowerBound + 1);
                            storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, maybeStates, maybeStates, false);
                            auto multiplier = storm::solver::MultiplierFactory<ValueType>().create(env, submatrix);
                            b = std::vector<ValueType>(b.size(), storm::utility::zero<ValueType>());
                            multiplier->repeatedMultiplyAndReduce(env, goal.direction(), subresult, &b, lowerBound - 1);
                        }
                    }
                    // Set the values of the resulting vector accordingly.
                    storm::utility::vector::setVectorValues(result, maybeStates, subresult);
                }
                if (lowerBound == 0) {
                    storm::utility::vector::setVectorValues(result, psiStates, storm::utility::one<ValueType>());
                }

                resultMaybeStates = maybeStates;
                return result;
            }

            template class SparseNondeterministicStepBoundedHorizonHelper<double>;
            template class SparseNondeterministicStepBoundedHorizonHelper<storm::RationalNumber>;
        }
    }
}