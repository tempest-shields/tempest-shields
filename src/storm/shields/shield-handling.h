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

#include "storm/io/file.h"
#include "storm/utility/macros.h"

#include "storm/exceptions/InvalidArgumentException.h"


namespace tempest {
    namespace shields {
        std::string shieldFilename(std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression) {
            std::stringstream filename;
            filename << shieldingExpression->typeToString() << "_";
            filename << shieldingExpression->comparisonToString();
            filename << shieldingExpression->getValue() << ".shield";
            return filename.str();
        }

        template<typename ValueType, typename IndexType = storm::storage::sparse::state_type>
        void createShield(std::shared_ptr<storm::models::sparse::Model<ValueType>> model, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates) {
            std::ofstream stream;
            storm::utility::openFile(shieldFilename(shieldingExpression), stream);
            if(shieldingExpression->isPreSafetyShield()) {
                PreSafetyShield<ValueType, IndexType> shield(model->getTransitionMatrix().getRowGroupIndices(), choiceValues, shieldingExpression, relevantStates, coalitionStates);
                shield.construct().printToStream(stream, model);
            } else if(shieldingExpression->isPostSafetyShield()) {
                PostSafetyShield<ValueType, IndexType> shield(model->getTransitionMatrix().getRowGroupIndices(), choiceValues, shieldingExpression, relevantStates, coalitionStates);
                shield.construct().printToStream(stream, model);
            } else if(shieldingExpression->isOptimalShield()) {
                storm::utility::closeFile(stream);
                STORM_LOG_THROW(false, storm::exceptions::InvalidArgumentException, "Cannot create optimal shields yet");
            }
            storm::utility::closeFile(stream);
        }
    }
}
