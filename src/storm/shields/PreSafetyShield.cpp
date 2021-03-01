#include "storm/shields/PreSafetyShield.h"

#include <algorithm>

namespace tempest {
    namespace shields {

        template<typename ValueType, typename IndexType>
        PreSafetyShield<ValueType, IndexType>::PreSafetyShield(std::vector<IndexType> const& rowGroupIndices, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates) : AbstractShield<ValueType, IndexType>(rowGroupIndices, choiceValues, shieldingExpression, relevantStates, coalitionStates) {
            // Intentionally left empty.
        }

        template<typename ValueType, typename IndexType>
        storm::storage::Scheduler<ValueType> PreSafetyShield<ValueType, IndexType>::construct() {
            storm::storage::Scheduler<ValueType> shield(this->rowGroupIndices.size() - 1);
            STORM_LOG_DEBUG(this->rowGroupIndices.size());
            STORM_LOG_DEBUG(this->relevantStates);
            for(auto const& x : this->choiceValues) {
                STORM_LOG_DEBUG(x << ",");
            }
            auto choice_it = this->choiceValues.begin();
            for(uint state = 0; state < this->rowGroupIndices.size() - 1; state++) {
                if(this->relevantStates.get(state)) {
                    uint rowGroupSize = this->rowGroupIndices[state + 1] - this->rowGroupIndices[state];
                    storm::storage::Distribution<ValueType, IndexType> actionDistribution;
                    ValueType maxProbability = *std::max_element(this->choiceValues.begin(), this->choiceValues.begin() + rowGroupSize);
                    for(uint choice = 0; choice < rowGroupSize; choice++, choice_it++) {
                        if(allowedValue<ValueType, IndexType>(maxProbability, *choice_it, this->shieldingExpression)) {
                            actionDistribution.addProbability(choice, *choice_it);
                        }
                    }
                    actionDistribution.normalize();
                    shield.setChoice(storm::storage::SchedulerChoice<ValueType>(actionDistribution), state);

                } else {
                    shield.setChoice(storm::storage::Distribution<ValueType, IndexType>(), state);
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
