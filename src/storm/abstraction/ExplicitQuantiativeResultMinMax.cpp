#include "storm/abstraction/ExplicitQuantitativeResultMinMax.h"

namespace storm {
    namespace abstraction {
        
        template<typename ValueType>
        ExplicitQuantitativeResultMinMax<ValueType>::ExplicitQuantitativeResultMinMax(uint64_t numberOfStates) = default;
        
        template<typename ValueType>
        ExplicitQuantitativeResult<ValueType> const& ExplicitQuantitativeResultMinMax<ValueType>::getMin() const {
            return min;
        }
        
        template<typename ValueType>
        ExplicitQuantitativeResult<ValueType>& ExplicitQuantitativeResultMinMax<ValueType>::getMin() {
            return min;
        }
        
        template<typename ValueType>
        ExplicitQuantitativeResult<ValueType> const& ExplicitQuantitativeResultMinMax<ValueType>::getMax() const {
            return max;
        }
        
        template<typename ValueType>
        ExplicitQuantitativeResult<ValueType>& ExplicitQuantitativeResultMinMax<ValueType>::getMax() {
            return max;
        }
        
        template<typename ValueType>
        ExplicitQuantitativeResult<ValueType> const& ExplicitQuantitativeResultMinMax<ValueType>::get(storm::OptimizationDirection const& dir) const {
            if (dir == storm::OptimizationDirection::Minimize) {
                return this->getMin();
            } else {
                return this->getMax();
            }
        }
        
        template<typename ValueType>
        ExplicitQuantitativeResult<ValueType>& ExplicitQuantitativeResultMinMax<ValueType>::get(storm::OptimizationDirection const& dir) {
            if (dir == storm::OptimizationDirection::Minimize) {
                return this->getMin();
            } else {
                return this->getMax();
            }
        }

    }
}
