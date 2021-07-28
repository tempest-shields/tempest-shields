#pragma once

#include "storm/shields/AbstractShield.h"
#include "storm/storage/OptimalScheduler.h"

namespace tempest {
    namespace shields {

        template<typename ValueType, typename IndexType>
        class OptimalShield : public AbstractShield<ValueType, IndexType> {
        public:
            OptimalShield(std::vector<IndexType> const& rowGroupIndices, std::vector<uint64_t> const& precomputedChoices, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates);

            storm::storage::OptimalScheduler<ValueType> construct();
        private:
            std::vector<uint64_t> precomputedChoices;
        };
    }
}
