#include "storm/shields/AbstractShield.h"

#include <boost/core/typeinfo.hpp>

namespace tempest {
    namespace shields {

        template<typename ValueType, typename IndexType>
        AbstractShield<ValueType, IndexType>::AbstractShield(std::vector<IndexType> const& rowGroupIndices, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates) : rowGroupIndices(rowGroupIndices), shieldingExpression(shieldingExpression), optimizationDirection(optimizationDirection), relevantStates(relevantStates), coalitionStates(coalitionStates) {
            // Intentionally left empty.
        }

        template<typename ValueType, typename IndexType>
        AbstractShield<ValueType, IndexType>::~AbstractShield() {
            // Intentionally left empty.
        }

        template<typename ValueType, typename IndexType>
        std::vector<IndexType> AbstractShield<ValueType, IndexType>::computeRowGroupSizes() {
            std::vector<IndexType> rowGroupSizes(this->rowGroupIndices.size() - 1);
            for(uint rowGroupStartIndex = 0; rowGroupStartIndex < rowGroupSizes.size(); rowGroupStartIndex++) {
                rowGroupSizes.at(rowGroupStartIndex) = this->rowGroupIndices[rowGroupStartIndex + 1] - this->rowGroupIndices[rowGroupStartIndex];
            }
            return rowGroupSizes;
        }

        template<typename ValueType, typename IndexType>
        storm::OptimizationDirection AbstractShield<ValueType, IndexType>::getOptimizationDirection() {
            return optimizationDirection;
        }

        template<typename ValueType, typename IndexType>
        std::string AbstractShield<ValueType, IndexType>::getClassName() const {
            return std::string(boost::core::demangled_name(BOOST_CORE_TYPEID(*this)));
        }

        // Explicitly instantiate appropriate
        template class AbstractShield<double, typename storm::storage::SparseMatrix<double>::index_type>;
#ifdef STORM_HAVE_CARL
        template class AbstractShield<storm::RationalNumber, typename storm::storage::SparseMatrix<storm::RationalNumber>::index_type>;
#endif
    }
}
