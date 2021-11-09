#pragma once

#include <iostream>
#include <boost/optional.hpp>
#include <memory>

#include "storm/storage/Scheduler.h"
#include "storm/storage/BitVector.h"

#include "storm/logic/ShieldExpression.h"

#include "storm/shields/AbstractShield.h"
#include "storm/shields/PreShield.h"
#include "storm/shields/PostShield.h"
#include "storm/shields/OptimalShield.h"

#include "storm/io/file.h"
#include "storm/utility/macros.h"

#include "storm/exceptions/InvalidArgumentException.h"

namespace tempest {
    namespace shields {
        std::string shieldFilename(std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression);

        template<typename ValueType, typename IndexType = storm::storage::sparse::state_type>
        void createShield(std::shared_ptr<storm::models::sparse::Model<ValueType>> model, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates);

        template<typename ValueType, typename IndexType = storm::storage::sparse::state_type>
        void createQuantitativeShield(std::shared_ptr<storm::models::sparse::Model<ValueType>> model, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates);
    }
}
