#pragma once

#include "storm/shields/AbstractShield.h"
#include "storm/storage/PreScheduler.h"

namespace tempest {
    namespace shields {

        template<typename ValueType, typename IndexType>
        class PreShield : public AbstractShield<ValueType, IndexType> {
        public:
            PreShield(std::vector<IndexType> const& rowGroupIndices, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates);

            storm::storage::PreScheduler<ValueType> construct();
            template<typename Compare, bool relative>
            storm::storage::PreScheduler<ValueType> constructWithCompareType();
        private:
            std::vector<ValueType> choiceValues;
        };
    }
}
