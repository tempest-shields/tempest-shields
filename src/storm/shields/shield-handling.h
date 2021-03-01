#pragma once

#include <iostream>
#include <boost/optional.hpp>
#include <memory>

#include "storm/storage/Scheduler.h"
#include "storm/storage/BitVector.h"

#include "storm/logic/ShieldExpression.h"

#include "storm/shields/AbstractShield.h"
#include "storm/shields/PreSafetyShield.h"

#include "storm/exceptions/InvalidArgumentException.h"


namespace tempest {
    namespace shields {
        template<typename ValueType, typename IndexType = storm::storage::sparse::state_type>
        storm::storage::Scheduler<ValueType> createShield(std::vector<IndexType> const& rowGroupIndices, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates) {
            if(shieldingExpression->isPreSafetyShield()) {
                PreSafetyShield<ValueType, IndexType> shield(rowGroupIndices, choiceValues, shieldingExpression, relevantStates, coalitionStates);
                return shield.construct();
            } else if(shieldingExpression->isPostSafetyShield()) {
                STORM_LOG_THROW(false, storm::exceptions::InvalidArgumentException, "Cannot create post safety shields yet");
            } else if(shieldingExpression->isOptimalShield()) {
                STORM_LOG_THROW(false, storm::exceptions::InvalidArgumentException, "Cannot create optimal shields yet");
            }
        }
    }
}
