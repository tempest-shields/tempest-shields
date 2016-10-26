#include "src/modelchecker/results/HybridQuantitativeCheckResult.h"
#include "src/modelchecker/results/SymbolicQualitativeCheckResult.h"
#include "src/modelchecker/results/ExplicitQuantitativeCheckResult.h"
#include "src/storage/dd/DdManager.h"
#include "src/storage/dd/cudd/CuddAddIterator.h"

#include "src/exceptions/InvalidOperationException.h"

#include "src/exceptions/NotImplementedException.h"
#include "src/utility/macros.h"
#include "src/utility/constants.h"


namespace storm {
    namespace modelchecker {
        template<storm::dd::DdType Type, typename ValueType>
        HybridQuantitativeCheckResult<Type, ValueType>::HybridQuantitativeCheckResult(storm::dd::Bdd<Type> const& reachableStates, storm::dd::Bdd<Type> const& symbolicStates, storm::dd::Add<Type, ValueType> const& symbolicValues, storm::dd::Bdd<Type> const& explicitStates, storm::dd::Odd const& odd, std::vector<ValueType> const& explicitValues) : reachableStates(reachableStates), symbolicStates(symbolicStates), symbolicValues(symbolicValues), explicitStates(explicitStates), odd(odd), explicitValues(explicitValues) {
            
            // Intentionally left empty.
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        std::unique_ptr<CheckResult> HybridQuantitativeCheckResult<Type, ValueType>::compareAgainstBound(storm::logic::ComparisonType comparisonType, ValueType const& bound) const {
            storm::dd::Bdd<Type> symbolicResult;

            // First compute the symbolic part of the result.
            if (comparisonType == storm::logic::ComparisonType::Less) {
                symbolicResult = symbolicValues.less(bound);
            } else if (comparisonType == storm::logic::ComparisonType::LessEqual) {
                symbolicResult = symbolicValues.lessOrEqual(bound);
            } else if (comparisonType == storm::logic::ComparisonType::Greater) {
                symbolicResult = symbolicValues.greater(bound);
            } else if (comparisonType == storm::logic::ComparisonType::GreaterEqual) {
                symbolicResult = symbolicValues.greaterOrEqual(bound);
            }
            
            // Then translate the explicit part to a symbolic format and simultaneously to a qualitative result.
            symbolicResult |= storm::dd::Bdd<Type>::fromVector(this->reachableStates.getDdManager(), this->explicitValues, this->odd, this->symbolicValues.getContainedMetaVariables(), comparisonType, bound);
            
            return std::unique_ptr<SymbolicQualitativeCheckResult<Type>>(new SymbolicQualitativeCheckResult<Type>(reachableStates, symbolicResult));
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        std::unique_ptr<CheckResult> HybridQuantitativeCheckResult<Type, ValueType>::toExplicitQuantitativeCheckResult() const {
            storm::dd::Bdd<Type> allStates = symbolicStates || explicitStates;
            storm::dd::Odd allStatesOdd = allStates.createOdd();
            
            std::vector<ValueType> fullExplicitValues = symbolicValues.toVector(allStatesOdd);
            this->odd.expandExplicitVector(allStatesOdd, this->explicitValues, fullExplicitValues);
            return std::unique_ptr<CheckResult>(new ExplicitQuantitativeCheckResult<ValueType>(std::move(fullExplicitValues)));
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        bool HybridQuantitativeCheckResult<Type, ValueType>::isHybrid() const {
            return true;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        bool HybridQuantitativeCheckResult<Type, ValueType>::isResultForAllStates() const {
            return (symbolicStates || explicitStates) == reachableStates;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        bool HybridQuantitativeCheckResult<Type, ValueType>::isHybridQuantitativeCheckResult() const {
            return true;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        storm::dd::Bdd<Type> const& HybridQuantitativeCheckResult<Type, ValueType>::getSymbolicStates() const {
            return symbolicStates;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        storm::dd::Add<Type, ValueType> const& HybridQuantitativeCheckResult<Type, ValueType>::getSymbolicValueVector() const {
            return symbolicValues;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        storm::dd::Bdd<Type> const& HybridQuantitativeCheckResult<Type, ValueType>::getExplicitStates() const {
            return explicitStates;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        storm::dd::Odd const& HybridQuantitativeCheckResult<Type, ValueType>::getOdd() const {
            return odd;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        std::vector<ValueType> const& HybridQuantitativeCheckResult<Type, ValueType>::getExplicitValueVector() const {
            return explicitValues;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        std::ostream& HybridQuantitativeCheckResult<Type, ValueType>::writeToStream(std::ostream& out) const {
            out << "[";
            bool first = true;
            if (!this->symbolicStates.isZero()) {
                if (this->symbolicValues.isZero()) {
                    out << "0";
                } else {
                    for (auto valuationValuePair : this->symbolicValues) {
                        if (!first) {
                            out << ", ";
                        } else {
                            first = false;
                        }
                        out << valuationValuePair.second;
                    }
                }
            }
            if (!this->explicitStates.isZero()) {
                for (auto const& element : this->explicitValues) {
                    if (!first) {
                        out << ", ";
                    } else {
                        first = false;
                    }
                    out << element;
                }
            }
            out << "]";
            return out;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        void HybridQuantitativeCheckResult<Type, ValueType>::filter(QualitativeCheckResult const& filter) {
            STORM_LOG_THROW(filter.isSymbolicQualitativeCheckResult(), storm::exceptions::InvalidOperationException, "Cannot filter hybrid check result with non-symbolic filter.");
            
            // First, we filter the symbolic values.
            this->symbolicStates = this->symbolicStates && filter.asSymbolicQualitativeCheckResult<Type>().getTruthValuesVector();
            this->symbolicValues *= symbolicStates.template toAdd<ValueType>();
            
            // Next, we filter the explicit values.

            // Start by computing the new set of states that is stored explictly and the corresponding ODD.
            this->explicitStates = this->explicitStates && filter.asSymbolicQualitativeCheckResult<Type>().getTruthValuesVector();
            storm::dd::Odd newOdd = explicitStates.createOdd();
            
            // Then compute the new vector of explicit values and set the new data fields.
            this->explicitValues = explicitStates.filterExplicitVector(this->odd, explicitValues);
            
            this->odd = newOdd;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        ValueType HybridQuantitativeCheckResult<Type, ValueType>::getMin() const {
            // In order to not get false zeros, we need to set the values of all states whose values is not stored
            // symbolically to infinity.
            storm::dd::Add<Type, ValueType> tmp = symbolicStates.ite(this->symbolicValues, reachableStates.getDdManager().getConstant(storm::utility::infinity<double>()));
            ValueType min = tmp.getMin();
            if (!explicitStates.isZero()) {
                for (auto const& element : explicitValues) {
                    min = std::min(min, element);
                }
            }
            return min;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        ValueType HybridQuantitativeCheckResult<Type, ValueType>::getMax() const {
            ValueType max = this->symbolicValues.getMax();
            if (!explicitStates.isZero()) {
                for (auto const& element : explicitValues) {
                    max = std::max(max, element);
                }
            }
            return max;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        ValueType HybridQuantitativeCheckResult<Type, ValueType>::sum() const {
            ValueType sum = symbolicValues.sumAbstract(symbolicValues.getContainedMetaVariables()).getValue();
            for (auto const& value : explicitValues) {
                sum += value;
            }
            return sum;
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        ValueType HybridQuantitativeCheckResult<Type, ValueType>::average() const {
            return this->sum() / (symbolicStates || explicitStates).getNonZeroCount();
        }
        
        template<storm::dd::DdType Type, typename ValueType>
        void HybridQuantitativeCheckResult<Type, ValueType>::oneMinus() {
            storm::dd::Add<Type> one = symbolicValues.getDdManager().template getAddOne<ValueType>();
            storm::dd::Add<Type> zero = symbolicValues.getDdManager().template getAddZero<ValueType>();
            symbolicValues = symbolicStates.ite(one - symbolicValues, zero);

            for (auto& element : explicitValues) {
                element = storm::utility::one<ValueType>() - element;
            }
        }
        
        
        // Explicitly instantiate the class.
        template class HybridQuantitativeCheckResult<storm::dd::DdType::CUDD>;
        template class HybridQuantitativeCheckResult<storm::dd::DdType::Sylvan>;
    }
}
