#include "storm/shields/AbstractShield.h"

#include <boost/core/typeinfo.hpp>
namespace tempest {
    namespace shields {

        template<typename ValueType, typename IndexType>
        AbstractShield<ValueType, IndexType>::AbstractShield(std::vector<IndexType> const& rowGroupIndices, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, boost::optional<storm::storage::BitVector> coalitionStates) : rowGroupIndices(rowGroupIndices), choiceValues(choiceValues), shieldingExpression(shieldingExpression), coalitionStates(coalitionStates) {
            // Intentionally left empty.
        }

        template<typename ValueType, typename IndexType>
        AbstractShield<ValueType, IndexType>::~AbstractShield() {
            // Intentionally left empty.
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
