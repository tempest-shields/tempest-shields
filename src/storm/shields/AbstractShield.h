#pragma once

#include <boost/optional.hpp>
#include <iostream>
#include <string>

#include "storm/storage/Scheduler.h"
#include "storm/storage/BitVector.h"

#include "storm/logic/ShieldExpression.h"

namespace tempest {
    namespace shields {

        template<typename ValueType, typename IndexType>
        class AbstractShield {
        public:
            typedef IndexType index_type;
            typedef ValueType value_type;

            virtual ~AbstractShield() = 0;

            /*!
             * TODO
             */
            virtual storm::storage::Scheduler<ValueType> construct() = 0;

            /*!
             * TODO
             */
            std::string getClassName() const;

        protected:
            AbstractShield(std::vector<IndexType> const& rowGroupIndices, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, boost::optional<storm::storage::BitVector> coalitionStates);

            std::vector<index_type> rowGroupIndices;
            std::vector<value_type> choiceValues;

            std::shared_ptr<storm::logic::ShieldExpression const> shieldingExpression;

            boost::optional<storm::storage::BitVector> coalitionStates;
        };
    }
}
