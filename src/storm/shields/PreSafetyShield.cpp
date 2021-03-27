#include "storm/shields/PreSafetyShield.h"

#include <algorithm>

namespace tempest {
    namespace shields {

        template<typename ValueType, typename IndexType>
        PreSafetyShield<ValueType, IndexType>::PreSafetyShield(std::vector<IndexType> const& rowGroupIndices, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates) : AbstractShield<ValueType, IndexType>(rowGroupIndices, choiceValues, shieldingExpression, optimizationDirection, relevantStates, coalitionStates) {
            // Intentionally left empty.
        }

        template<typename ValueType, typename IndexType>
        storm::storage::Scheduler<ValueType> PreSafetyShield<ValueType, IndexType>::construct() {
            if (this->getOptimizationDirection() == storm::OptimizationDirection::Minimize) {
                if(this->shieldingExpression->isRelative()) {
                    return constructWithCompareType<storm::utility::ElementLessEqual<ValueType>, true>();
                } else {
                    return constructWithCompareType<storm::utility::ElementLessEqual<ValueType>, false>();
                }
            } else {
                if(this->shieldingExpression->isRelative()) {
                    return constructWithCompareType<storm::utility::ElementGreaterEqual<ValueType>, true>();
                } else {
                    return constructWithCompareType<storm::utility::ElementGreaterEqual<ValueType>, false>();
                }
            }
        }

        template<typename ValueType, typename IndexType>
        template<typename Compare, bool relative>
        storm::storage::Scheduler<ValueType> PreSafetyShield<ValueType, IndexType>::constructWithCompareType() {
            tempest::shields::utility::ChoiceFilter<ValueType, Compare, relative> choiceFilter;
            storm::storage::Scheduler<ValueType> shield(this->rowGroupIndices.size() - 1);
            auto choice_it = this->choiceValues.begin();
            if(this->coalitionStates.is_initialized()) {
                this->relevantStates &= this->coalitionStates.get();
            }
            for(uint state = 0; state < this->rowGroupIndices.size() - 1; state++) {
                uint rowGroupSize = this->rowGroupIndices[state + 1] - this->rowGroupIndices[state];
                if(this->relevantStates.get(state)) {
                    storm::storage::Distribution<ValueType, IndexType> actionDistribution;
                    ValueType maxProbability = *std::max_element(choice_it, choice_it + rowGroupSize);
                    if(!relative && !choiceFilter(maxProbability, maxProbability, this->shieldingExpression->getValue())) {
                        STORM_LOG_WARN("No shielding action possible with absolute comparison for state with index " << state);
                        shield.setChoice(storm::storage::Distribution<ValueType, IndexType>(), state);
                        choice_it += rowGroupSize;
                        continue;
                    }
                    for(uint choice = 0; choice < rowGroupSize; choice++, choice_it++) {
                        if(choiceFilter(*choice_it, maxProbability, this->shieldingExpression->getValue())) {
                            actionDistribution.addProbability(choice, *choice_it);
                        }
                    }
                    shield.setChoice(storm::storage::SchedulerChoice<ValueType>(actionDistribution), state);

                } else {
                    shield.setChoice(storm::storage::Distribution<ValueType, IndexType>(), state);
                    choice_it += rowGroupSize;
                }
            }
            return shield;
        }
        // Explicitly instantiate appropriate
        template class PreSafetyShield<double, typename storm::storage::SparseMatrix<double>::index_type>;
#ifdef STORM_HAVE_CARL
        template class PreSafetyShield<storm::RationalNumber, typename storm::storage::SparseMatrix<storm::RationalNumber>::index_type>;
#endif
    }
}
