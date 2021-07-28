#include "BoundedGloballyGameViHelper.h"

#include "storm/environment/Environment.h"
#include "storm/environment/solver/SolverEnvironment.h"
#include "storm/environment/solver/GameSolverEnvironment.h"

#include "storm/utility/SignalHandler.h"
#include "storm/utility/vector.h"

namespace storm {
    namespace modelchecker {
        namespace helper {
            namespace internal {

                template <typename ValueType>
                BoundedGloballyGameViHelper<ValueType>::BoundedGloballyGameViHelper(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::BitVector statesOfCoalition) : _transitionMatrix(transitionMatrix), _statesOfCoalition(statesOfCoalition) {
                }

                template <typename ValueType>
                void BoundedGloballyGameViHelper<ValueType>::prepareSolversAndMultipliers(const Environment& env) {
                    _multiplier = storm::solver::MultiplierFactory<ValueType>().create(env, _transitionMatrix);
                }

                template <typename ValueType>
                void BoundedGloballyGameViHelper<ValueType>::performValueIteration(Environment const& env, std::vector<ValueType>& x, storm::solver::OptimizationDirection const dir, uint64_t upperBound, std::vector<ValueType>& constrainedChoiceValues) {
                    prepareSolversAndMultipliers(env);
                    _x = x;

                    if (this->isProduceSchedulerSet()) {
                        if (!this->_producedOptimalChoices.is_initialized()) {
                            this->_producedOptimalChoices.emplace();
                        }
                        this->_producedOptimalChoices->resize(this->_transitionMatrix.getRowGroupCount());
                    }

                    for (uint64_t iter = 0; iter < upperBound; iter++) {
                        if(iter == upperBound - 1) {
                            _multiplier->multiply(env, _x, nullptr, constrainedChoiceValues);
                        }
                        performIterationStep(env, dir);
                    }

                    x = _x;
                }

                template <typename ValueType>
                void BoundedGloballyGameViHelper<ValueType>::performIterationStep(Environment const& env, storm::solver::OptimizationDirection const dir, std::vector<uint64_t>* choices) {
                    if (!_multiplier) {
                        prepareSolversAndMultipliers(env);
                    }

                    // multiplyandreducegaussseidel
                    // Ax = x'
                    if (choices == nullptr) {
                        _multiplier->multiplyAndReduce(env, dir, _x, nullptr, _x, nullptr, &_statesOfCoalition);
                    } else {
                        // Also keep track of the choices made.
                        _multiplier->multiplyAndReduce(env, dir, _x, nullptr, _x, choices, &_statesOfCoalition);
                    }
                }

                template <typename ValueType>
                void BoundedGloballyGameViHelper<ValueType>::fillResultVector(std::vector<ValueType>& result, storm::storage::BitVector psiStates)
                {
                    std::vector<ValueType> filledVector = std::vector<ValueType>(psiStates.size(), storm::utility::zero<ValueType>());
                    uint bitIndex = 0;
                    for(uint i = 0; i < filledVector.size(); i++) {
                        if (psiStates.get(i)) {
                            filledVector.at(i) = result.at(bitIndex);
                            bitIndex++;
                        }
                    }
                    result = filledVector;
                }

                template <typename ValueType>
                void BoundedGloballyGameViHelper<ValueType>::fillChoiceValuesVector(std::vector<ValueType>& choiceValues, storm::storage::BitVector psiStates, std::vector<storm::storage::SparseMatrix<double>::index_type> rowGroupIndices) {
                    std::vector<ValueType> allChoices = std::vector<ValueType>(rowGroupIndices.at(rowGroupIndices.size() - 1), storm::utility::zero<ValueType>());
                    auto choice_it = choiceValues.begin();
                    for(uint state = 0; state < rowGroupIndices.size() - 1; state++) {
                        uint rowGroupSize = rowGroupIndices[state + 1] - rowGroupIndices[state];
                        if (psiStates.get(state)) {
                            for(uint choice = 0; choice < rowGroupSize; choice++, choice_it++) {
                                allChoices.at(rowGroupIndices.at(state) + choice) = *choice_it;
                            }
                        }
                    }
                    choiceValues = allChoices;
                }

                template <typename ValueType>
                void BoundedGloballyGameViHelper<ValueType>::setProduceScheduler(bool value) {
                    _produceScheduler = value;
                }

                template <typename ValueType>
                bool BoundedGloballyGameViHelper<ValueType>::isProduceSchedulerSet() const {
                    return _produceScheduler;
                }

                template <typename ValueType>
                std::vector<uint64_t> const& BoundedGloballyGameViHelper<ValueType>::getProducedOptimalChoices() const {
                    STORM_LOG_ASSERT(this->isProduceSchedulerSet(), "Trying to get the produced optimal choices although no scheduler was requested.");
                    STORM_LOG_ASSERT(this->_producedOptimalChoices.is_initialized(), "Trying to get the produced optimal choices but none were available. Was there a computation call before?");
                    return this->_producedOptimalChoices.get();
                }

                template <typename ValueType>
                std::vector<uint64_t>& BoundedGloballyGameViHelper<ValueType>::getProducedOptimalChoices() {
                    STORM_LOG_ASSERT(this->isProduceSchedulerSet(), "Trying to get the produced optimal choices although no scheduler was requested.");
                    STORM_LOG_ASSERT(this->_producedOptimalChoices.is_initialized(), "Trying to get the produced optimal choices but none were available. Was there a computation call before?");
                    return this->_producedOptimalChoices.get();
                }

                template <typename ValueType>
                storm::storage::Scheduler<ValueType> BoundedGloballyGameViHelper<ValueType>::extractScheduler() const{
                    auto const& optimalChoices = getProducedOptimalChoices();
                    storm::storage::Scheduler<ValueType> scheduler(optimalChoices.size());
                    for (uint64_t state = 0; state < optimalChoices.size(); ++state) {
                        scheduler.setChoice(optimalChoices[state], state);
                    }
                    return scheduler;
                }

                template class BoundedGloballyGameViHelper<double>;
#ifdef STORM_HAVE_CARL
                template class BoundedGloballyGameViHelper<storm::RationalNumber>;
#endif
            }
        }
    }
}
