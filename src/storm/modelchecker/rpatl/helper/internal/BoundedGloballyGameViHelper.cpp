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
                BoundedGloballyGameViHelper<ValueType>::GameViHelper(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::BitVector statesOfCoalition) : _transitionMatrix(transitionMatrix), _statesOfCoalition(statesOfCoalition) {
                }

                template <typename ValueType>
                void BoundedGloballyGameViHelper<ValueType>::prepareSolversAndMultipliersReachability(const Environment& env) {
                    _multiplier = storm::solver::MultiplierFactory<ValueType>().create(env, _transitionMatrix);
                    _x1IsCurrent = false;
                }

                template <typename ValueType>
                void BoundedGloballyGameViHelper<ValueType>::performValueIteration(Environment const& env, std::vector<ValueType>& x, std::vector<ValueType> b, storm::solver::OptimizationDirection const dir) {
                    prepareSolversAndMultipliersReachability(env);
                    // TODO: parse k from the formula
                    uint64_t k = 3;
                    _b = b;

                    _x1.assign(_transitionMatrix.getRowGroupCount(), storm::utility::zero<ValueType>());
                    _x2 = _x1;

                    if (this->isProduceSchedulerSet()) {
                        if (!this->_producedOptimalChoices.is_initialized()) {
                            this->_producedOptimalChoices.emplace();
                        }
                        this->_producedOptimalChoices->resize(this->_transitionMatrix.getRowGroupCount());
                    }

                    uint64_t iter = 0;
                    std::vector<ValueType> result = x;
                    while (iter < k) {
                        ++iter;
                        performIterationStep(env, dir);

                        if (storm::utility::resources::isTerminate()) {
                            break;
                        }
                    }
                    x = xNew();

                    // TODO: do we have to do that by using boundedGlobally?
                    if (isProduceSchedulerSet()) {
                        // We will be doing one more iteration step and track scheduler choices this time.
                        performIterationStep(env, dir, &_producedOptimalChoices.get());
                    }
                }

                template <typename ValueType>
                void BoundedGloballyGameViHelper<ValueType>::performIterationStep(Environment const& env, storm::solver::OptimizationDirection const dir, std::vector<uint64_t>* choices) {
                    if (!_multiplier) {
                        prepareSolversAndMultipliersReachability(env);
                    }
                    _x1IsCurrent = !_x1IsCurrent;

                    // multiplyandreducegaussseidel
                    // Ax + b
                    if (choices == nullptr) {
                        //STORM_LOG_DEBUG("\n" << _transitionMatrix);
                        //STORM_LOG_DEBUG("xOld = " << storm::utility::vector::toString(xOld()));
                        //STORM_LOG_DEBUG("b = " << storm::utility::vector::toString(_b));
                        //STORM_LOG_DEBUG("xNew = " << storm::utility::vector::toString(xNew()));
                        _multiplier->multiplyAndReduce(env, dir, xOld(), &_b, xNew(), nullptr, &_statesOfCoalition);
                        //std::cin.get();
                    } else {
                        // Also keep track of the choices made.
                        _multiplier->multiplyAndReduce(env, dir, xOld(), &_b, xNew(), choices, &_statesOfCoalition);
                    }

                    // compare applyPointwise
                    storm::utility::vector::applyPointwise<ValueType, ValueType, ValueType>(xOld(), xNew(), xNew(), [&dir] (ValueType const& a, ValueType const& b) -> ValueType {
                        if(storm::solver::maximize(dir)) {
                            if(a > b) return a;
                            else return b;
                        } else {
                            if(a > b) return a;
                            else return b;
                        }
                    });
                }

                template <typename ValueType>
                void BoundedGloballyGameViHelper<ValueType>::fillResultVector(std::vector<ValueType>& result, storm::storage::BitVector psiStates)
                {
                    std::vector<ValueType> filledVector = std::vector<ValueType>(relevantStates.size(), storm::utility::zero<ValueType>());
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

                template <typename ValueType>
                std::vector<ValueType>& BoundedGloballyGameViHelper<ValueType>::xNew() {
                    return _x1IsCurrent ? _x1 : _x2;
                }

                template <typename ValueType>
                std::vector<ValueType> const& BoundedGloballyGameViHelper<ValueType>::xNew() const {
                    return _x1IsCurrent ? _x1 : _x2;
                }

                template <typename ValueType>
                std::vector<ValueType>& BoundedGloballyGameViHelper<ValueType>::xOld() {
                    return _x1IsCurrent ? _x2 : _x1;
                }

                template <typename ValueType>
                std::vector<ValueType> const& BoundedGloballyGameViHelper<ValueType>::xOld() const {
                    return _x1IsCurrent ? _x2 : _x1;
                }

                template class BoundedGloballyGameViHelper<double>;
#ifdef STORM_HAVE_CARL
                template class BoundedGloballyGameViHelper<storm::RationalNumber>;
#endif
            }
        }
    }
}
