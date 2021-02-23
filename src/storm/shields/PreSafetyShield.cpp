#include "storm/shields/PreSafetyShield.h"

namespace tempest {
    namespace shields {

        template<typename ValueType, typename IndexType>
        PreSafetyShield<ValueType, IndexType>::PreSafetyShield(std::vector<IndexType> const& rowGroupIndices, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, boost::optional<storm::storage::BitVector> coalitionStates) : AbstractShield<ValueType, IndexType>(rowGroupIndices, choiceValues, shieldingExpression, coalitionStates) {
            // Intentionally left empty.
        }

        template<typename ValueType, typename IndexType>
        storm::storage::Scheduler<ValueType> PreSafetyShield<ValueType, IndexType>::construct() {
            for(auto const& x: this->rowGroupIndices) {
                STORM_LOG_DEBUG(x << ", ");
            }
            for(auto const& x: this->choiceValues) {
                STORM_LOG_DEBUG(x << ", ");
            }
            STORM_LOG_ASSERT(false, "construct NYI");
        }
        // Explicitly instantiate appropriate
        template class PreSafetyShield<double, typename storm::storage::SparseMatrix<double>::index_type>;
        template class PreSafetyShield<float, typename storm::storage::SparseMatrix<float>::index_type>;
        template class PreSafetyShield<int, typename storm::storage::SparseMatrix<int>::index_type>;
        template class PreSafetyShield<storm::storage::sparse::state_type, typename storm::storage::SparseMatrix<storm::storage::sparse::state_type>::index_type>;
#ifdef STORM_HAVE_CARL
        template class PreSafetyShield<storm::RationalNumber, typename storm::storage::SparseMatrix<storm::RationalNumber>::index_type>;
#endif
    }
}
