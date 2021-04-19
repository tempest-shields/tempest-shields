#include "storm/shields/OptimalShield.h"

#include <algorithm>

namespace tempest {
    namespace shields {

        template<typename ValueType, typename IndexType>
        OptimalShield<ValueType, IndexType>::OptimalShield(std::vector<IndexType> const& rowGroupIndices, std::vector<uint64_t> const& precomputedChoices, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates) : AbstractShield<ValueType, IndexType>(rowGroupIndices, shieldingExpression, optimizationDirection, relevantStates, coalitionStates), precomputedChoices(precomputedChoices) {
            // Intentionally left empty.
        }

        template<typename ValueType, typename IndexType>
        storm::storage::OptimalScheduler<ValueType> OptimalShield<ValueType, IndexType>::construct() {
            storm::storage::OptimalScheduler<ValueType> shield(this->rowGroupIndices.size() - 1);
            // TODO Needs fixing as soon as we support MDPs
            if(this->coalitionStates.is_initialized()) {
                this->relevantStates = ~this->relevantStates;
            }
            for(uint state = 0; state < this->rowGroupIndices.size() - 1; state++) {
                if(this->relevantStates.get(state)) {
                    shield.setChoice(precomputedChoices[state], state);
                } else {
                    shield.setChoice(storm::storage::Distribution<ValueType, IndexType>(), state);
                }
            }
            return shield;
        }
        // Explicitly instantiate appropriate
        template class OptimalShield<double, typename storm::storage::SparseMatrix<double>::index_type>;
#ifdef STORM_HAVE_CARL
        template class OptimalShield<storm::RationalNumber, typename storm::storage::SparseMatrix<storm::RationalNumber>::index_type>;
#endif
    }
}
