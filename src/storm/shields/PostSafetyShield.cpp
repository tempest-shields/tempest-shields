#include "storm/shields/PostSafetyShield.h"

#include <algorithm>

namespace tempest {
    namespace shields {

        template<typename ValueType, typename IndexType>
        PostSafetyShield<ValueType, IndexType>::PostSafetyShield(std::vector<IndexType> const& rowGroupIndices, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates) : AbstractShield<ValueType, IndexType>(rowGroupIndices, choiceValues, shieldingExpression, relevantStates, coalitionStates) {
            // Intentionally left empty.
        }

        template<typename ValueType, typename IndexType>
        storm::storage::PostScheduler<ValueType> PostSafetyShield<ValueType, IndexType>::construct() {
            storm::storage::PostScheduler<ValueType> shield(this->rowGroupIndices.size() - 1, this->computeRowGroupSizes());
            STORM_LOG_DEBUG(this->rowGroupIndices.size());
            STORM_LOG_DEBUG(this->relevantStates);
            STORM_LOG_DEBUG(this->coalitionStates.get());
            for(auto const& x : this->choiceValues) {
                STORM_LOG_DEBUG(x << ",");
            }
            auto choice_it = this->choiceValues.begin();
            if(this->coalitionStates.is_initialized()) {
                this->relevantStates &= this->coalitionStates.get();
            }
            STORM_LOG_DEBUG(this->relevantStates);
            for(uint state = 0; state < this->rowGroupIndices.size() - 1; state++) {
                if(this->relevantStates.get(state)) {
                    uint rowGroupSize = this->rowGroupIndices[state + 1] - this->rowGroupIndices[state];
                    auto maxProbabilityIndex = std::max_element(choice_it, choice_it + rowGroupSize) - choice_it;
                    ValueType maxProbability = *(choice_it + maxProbabilityIndex);
                    for(uint choice = 0; choice < rowGroupSize; choice++, choice_it++) {
                        STORM_LOG_DEBUG("processing " << state << " with rowGroupSize of " << rowGroupSize << " choice index " << choice << " prob: " << *choice_it << " > " << maxProbability);
                        storm::storage::Distribution<ValueType, IndexType> actionDistribution;
                        if(allowedValue<ValueType, IndexType>(maxProbability, *choice_it, this->shieldingExpression)) {
                            actionDistribution.addProbability(choice, 1);
                        } else {
                            actionDistribution.addProbability(maxProbabilityIndex, 1);
                        }
                        actionDistribution.normalize();
                        STORM_LOG_DEBUG("  dist: " << actionDistribution);
                        shield.setChoice(choice, storm::storage::SchedulerChoice<ValueType>(actionDistribution), state);
                    }
                } else {
                    shield.setChoice(0, storm::storage::Distribution<ValueType, IndexType>(), state);
                }
            }
            return shield;
        }

        // Explicitly instantiate appropriate
        template class PostSafetyShield<double, typename storm::storage::SparseMatrix<double>::index_type>;
#ifdef STORM_HAVE_CARL
        template class PostSafetyShield<storm::RationalNumber, typename storm::storage::SparseMatrix<storm::RationalNumber>::index_type>;
#endif
    }
}
