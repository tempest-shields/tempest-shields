#pragma once

#include <iostream>
#include <boost/optional.hpp>
#include <memory>

#include "storm/storage/Scheduler.h"
#include "storm/storage/BitVector.h"

#include "storm/logic/ShieldExpression.h"

#include "storm/shields/AbstractShield.h"
#include "storm/shields/PreSafetyShield.h"
#include "storm/shields/PostSafetyShield.h"
#include "storm/shields/OptimalShield.h"

#include "storm/io/file.h"
#include "storm/utility/macros.h"

#include "storm/exceptions/InvalidArgumentException.h"


namespace tempest {
    namespace shields {
        std::string shieldFilename(std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression) {
            return shieldingExpression->getFilename() + ".shield";
        }

        template<typename ValueType, typename IndexType = storm::storage::sparse::state_type>
        void createShield(std::shared_ptr<storm::models::sparse::Model<ValueType>> model, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates) {
            std::ofstream stream;
            storm::utility::openFile(shieldFilename(shieldingExpression), stream);
            if(shieldingExpression->isPreSafetyShield()) {
                PreSafetyShield<ValueType, IndexType> shield(model->getTransitionMatrix().getRowGroupIndices(), choiceValues, shieldingExpression, optimizationDirection, relevantStates, coalitionStates);
                shield.construct().printToStream(stream, shieldingExpression, model);
            } else if(shieldingExpression->isPostSafetyShield()) {
                PostSafetyShield<ValueType, IndexType> shield(model->getTransitionMatrix().getRowGroupIndices(), choiceValues, shieldingExpression, optimizationDirection, relevantStates, coalitionStates);
                shield.construct().printToStream(stream, shieldingExpression, model);
            } else {
                STORM_LOG_THROW(false, storm::exceptions::InvalidArgumentException, "Cannot create " + shieldingExpression->typeToString() + " shields yet");
                storm::utility::closeFile(stream);
            }
            storm::utility::closeFile(stream);
        }

        template<typename ValueType, typename IndexType = storm::storage::sparse::state_type>
        void createOptimalShield(std::shared_ptr<storm::models::sparse::Model<ValueType>> model, std::vector<uint64_t> const& precomputedChoices, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates) {
            std::ofstream stream;
            storm::utility::openFile(shieldFilename(shieldingExpression), stream);
            if(shieldingExpression->isOptimalShield()) {
                STORM_LOG_DEBUG("createOptimalShield");
                OptimalShield<ValueType, IndexType> shield(model->getTransitionMatrix().getRowGroupIndices(), precomputedChoices, shieldingExpression, optimizationDirection, relevantStates, coalitionStates);
                shield.construct().printToStream(stream, shieldingExpression, model);
            } else {
                STORM_LOG_THROW(false, storm::exceptions::InvalidArgumentException, "Cannot create " + shieldingExpression->typeToString() + " shields yet");
                storm::utility::closeFile(stream);
            }
            storm::utility::closeFile(stream);
        }
    }
}
