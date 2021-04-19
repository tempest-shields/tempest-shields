#pragma once

#include <boost/optional.hpp>
#include <iostream>
#include <string>
#include <memory>

#include "storm/storage/Scheduler.h"
#include "storm/storage/SchedulerChoice.h"
#include "storm/storage/BitVector.h"
#include "storm/storage/Distribution.h"

#include "storm/utility/constants.h"

#include "storm/solver/OptimizationDirection.h"

#include "storm/logic/ShieldExpression.h"

namespace tempest {
    namespace shields {
        namespace utility {
            template<typename ValueType, typename Compare, bool relative>
            struct ChoiceFilter {
                bool operator()(ValueType v, ValueType max, double shieldValue) {
                    Compare compare;
                    if(relative) return compare(v, max * shieldValue);
                    else return compare(v, shieldValue);
                }
            };
        }

        template<typename ValueType, typename IndexType>
        class AbstractShield {
        public:
            typedef IndexType index_type;
            typedef ValueType value_type;

            virtual ~AbstractShield() = 0;

            /*!
             * TODO
             */
            std::vector<IndexType> computeRowGroupSizes();

            /*!
             * TODO
             */
            bool allowedValue(ValueType const& max, ValueType const& v, std::shared_ptr<storm::logic::ShieldExpression const> const shieldExpression);

            storm::OptimizationDirection getOptimizationDirection();

            /*!
             * TODO
             */
            std::string getClassName() const;

        protected:
            AbstractShield(std::vector<IndexType> const& rowGroupIndices, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates);

            std::vector<index_type> rowGroupIndices;
            //std::vector<value_type> choiceValues;

            std::shared_ptr<storm::logic::ShieldExpression const> shieldingExpression;
            storm::OptimizationDirection optimizationDirection;

            storm::storage::BitVector relevantStates;
            boost::optional<storm::storage::BitVector> coalitionStates;
        };
    }
}
