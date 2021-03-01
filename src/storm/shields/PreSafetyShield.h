#pragma once

#include "storm/shields/AbstractShield.h"

namespace tempest {
    namespace shields {

        template<typename ValueType, typename IndexType>
        class PreSafetyShield : public AbstractShield<ValueType, IndexType> {
        public:
            PreSafetyShield(std::vector<IndexType> const& rowGroupIndices, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates);
            storm::storage::Scheduler<ValueType> construct();
        };
    }
}
