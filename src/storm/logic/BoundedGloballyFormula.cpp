#include "storm/logic/BoundedGloballyFormula.h"

#include "storm/utility/constants.h"
#include "storm/utility/macros.h"
#include "storm/exceptions/InvalidArgumentException.h"

#include "storm/logic/FormulaVisitor.h"

#include "storm/exceptions/InvalidPropertyException.h"
#include "storm/exceptions/InvalidOperationException.h"

namespace storm {
    namespace logic {
        BoundedGloballyFormula::BoundedGloballyFormula(std::shared_ptr<Formula const> const& subformula, boost::optional<TimeBound> const& lowerBound, boost::optional<TimeBound> const& upperBound, TimeBoundReference const& timeBoundReference) : UnaryPathFormula(subformula), subformula({subformula}), timeBoundReference({timeBoundReference}), lowerBound({lowerBound}), upperBound({upperBound}) {
            STORM_LOG_THROW(lowerBound || upperBound, storm::exceptions::InvalidArgumentException, "Bounded until formula requires at least one bound.");
        }

        BoundedGloballyFormula::BoundedGloballyFormula(std::shared_ptr<Formula const> const& subformula, std::vector<boost::optional<TimeBound>> const& lowerBounds, std::vector<boost::optional<TimeBound>> const& upperBounds, std::vector<TimeBoundReference> const& timeBoundReferences) : UnaryPathFormula(subformula), subformula({subformula}), timeBoundReference(timeBoundReferences), lowerBound(lowerBounds), upperBound(upperBounds) {
            assert(timeBoundReferences.size() == upperBound.size());
            assert(timeBoundReferences.size() == lowerBound.size());
        }

        // TODO handle the input for a vector of subformulas to UnaryPathFormula
        BoundedGloballyFormula::BoundedGloballyFormula(std::vector<std::shared_ptr<Formula const>> const& subformulas, std::vector<boost::optional<TimeBound>> const& lowerBounds, std::vector<boost::optional<TimeBound>> const& upperBounds, std::vector<TimeBoundReference> const& timeBoundReferences) : UnaryPathFormula(subformulas.at(0)), subformula({subformula}), timeBoundReference(timeBoundReferences), lowerBound(lowerBounds), upperBound(upperBounds) {
            assert(subformula.size() == timeBoundReference.size());
            assert(timeBoundReference.size() == lowerBound.size());
            assert(lowerBound.size() == upperBound.size());
            STORM_LOG_THROW(this->getDimension() != 0, storm::exceptions::InvalidArgumentException, "Bounded until formula requires at least one dimension.");
            for (unsigned i = 0; i < timeBoundReferences.size(); ++i) {
                STORM_LOG_THROW(hasLowerBound(i) || hasUpperBound(i), storm::exceptions::InvalidArgumentException, "Bounded until formula requires at least one bound in each dimension.");
            }
        }

        bool BoundedGloballyFormula::isBoundedGloballyFormula() const {
            return true;
        }

        bool BoundedGloballyFormula::isProbabilityPathFormula() const {
            return true;
        }

        boost::any BoundedGloballyFormula::accept(FormulaVisitor const& visitor, boost::any const& data) const {
            return visitor.visit(*this, data);
        }

        bool BoundedGloballyFormula::isMultiDimensional() const {
            assert(timeBoundReference.size() != 0);
            return timeBoundReference.size() > 1;
        }

        bool BoundedGloballyFormula::hasMultiDimensionalSubformulas() const {
            assert(subformula.size() != 0);
            return subformula.size() > 1;
        }

        unsigned BoundedGloballyFormula::getDimension() const {
            return timeBoundReference.size();
        }

        Formula const& BoundedGloballyFormula::getSubformula() const {
            STORM_LOG_ASSERT(subformula.size() == 1, "The subformula is not unique.");
            return *subformula.at(0);
        }

        Formula const& BoundedGloballyFormula::getSubformula(unsigned i) const {
            if (subformula.size() == 1 && i < getDimension()) {
                return getSubformula();
            } else {
                return *subformula.at(i);
            }
        }

        TimeBoundReference const& BoundedGloballyFormula::getTimeBoundReference(unsigned i) const {
            assert(i < timeBoundReference.size());
            return timeBoundReference.at(i);
        }

        bool BoundedGloballyFormula::isLowerBoundStrict(unsigned i) const {
            assert(i < lowerBound.size());
            if (!hasLowerBound(i)) { return false; }
            return lowerBound.at(i).get().isStrict();
        }

        bool BoundedGloballyFormula::hasLowerBound() const {
            for(auto const& lb : lowerBound) {
                if (static_cast<bool>(lb)) {
                    return true;
                }
            }
            return false;
        }

        bool BoundedGloballyFormula::hasLowerBound(unsigned i) const {
            return static_cast<bool>(lowerBound.at(i));
        }

        bool BoundedGloballyFormula::hasIntegerLowerBound(unsigned i) const {
            if (!hasLowerBound(i)) { return true; }
            return lowerBound.at(i).get().getBound().hasIntegerType();
        }

        bool BoundedGloballyFormula::isUpperBoundStrict(unsigned i) const {
            return upperBound.at(i).get().isStrict();
        }

        bool BoundedGloballyFormula::hasUpperBound() const {
            for (auto const& ub : upperBound) {
                if (static_cast<bool>(ub)) {
                    return true;
                }
            }
            return false;
        }

        bool BoundedGloballyFormula::hasUpperBound(unsigned i) const {
            return static_cast<bool>(upperBound.at(i));
        }

        bool BoundedGloballyFormula::hasIntegerUpperBound(unsigned i) const {
            return upperBound.at(i).get().getBound().hasIntegerType();
        }

        storm::expressions::Expression const& BoundedGloballyFormula::getLowerBound(unsigned i) const {
            return lowerBound.at(i).get().getBound();
        }

        storm::expressions::Expression const& BoundedGloballyFormula::getUpperBound(unsigned i) const {
            return upperBound.at(i).get().getBound();
        }

        template <>
        double BoundedGloballyFormula::getLowerBound(unsigned i) const {
            if (!hasLowerBound(i)) { return 0.0; }
            checkNoVariablesInBound(this->getLowerBound());
            double bound = this->getLowerBound(i).evaluateAsDouble();
            STORM_LOG_THROW(bound >= 0, storm::exceptions::InvalidPropertyException, "Time-bound must not evaluate to negative number.");
            return bound;
        }

        template <>
        double BoundedGloballyFormula::getUpperBound(unsigned i) const {
            checkNoVariablesInBound(this->getUpperBound());
            double bound = this->getUpperBound(i).evaluateAsDouble();
            STORM_LOG_THROW(bound >= 0, storm::exceptions::InvalidPropertyException, "Time-bound must not evaluate to negative number.");
            return bound;
        }

        template <>
        storm::RationalNumber BoundedGloballyFormula::getLowerBound(unsigned i) const {
            if (!hasLowerBound(i)) { return storm::utility::zero<storm::RationalNumber>(); }
            checkNoVariablesInBound(this->getLowerBound(i));
            storm::RationalNumber bound = this->getLowerBound(i).evaluateAsRational();
            STORM_LOG_THROW(bound >= storm::utility::zero<storm::RationalNumber>(), storm::exceptions::InvalidPropertyException, "Time-bound must not evaluate to negative number.");
            return bound;
        }

        template <>
        storm::RationalNumber BoundedGloballyFormula::getUpperBound(unsigned i) const {
            checkNoVariablesInBound(this->getUpperBound(i));
            storm::RationalNumber bound = this->getUpperBound(i).evaluateAsRational();
            STORM_LOG_THROW(bound >= storm::utility::zero<storm::RationalNumber>(), storm::exceptions::InvalidPropertyException, "Time-bound must not evaluate to negative number.");
            return bound;
        }

        template <>
        uint64_t BoundedGloballyFormula::getLowerBound(unsigned i) const {
            if (!hasLowerBound(i)) { return 0; }
            checkNoVariablesInBound(this->getLowerBound(i));
            int_fast64_t bound = this->getLowerBound(i).evaluateAsInt();
            STORM_LOG_THROW(bound >= 0, storm::exceptions::InvalidPropertyException, "Time-bound must not evaluate to negative number.");
            return static_cast<uint64_t>(bound);
        }

        template <>
        uint64_t BoundedGloballyFormula::getUpperBound(unsigned i) const {
            checkNoVariablesInBound(this->getUpperBound(i));
            int_fast64_t bound = this->getUpperBound(i).evaluateAsInt();
            STORM_LOG_THROW(bound >= 0, storm::exceptions::InvalidPropertyException, "Time-bound must not evaluate to negative number.");
            return static_cast<uint64_t>(bound);
        }

        template <>
        double BoundedGloballyFormula::getNonStrictUpperBound(unsigned i) const {
            double bound = getUpperBound<double>(i);
            STORM_LOG_THROW(!isUpperBoundStrict(i) || bound > 0, storm::exceptions::InvalidPropertyException, "Cannot retrieve non-strict bound from strict zero-bound.");
            return bound;
        }

        template <>
        uint64_t BoundedGloballyFormula::getNonStrictUpperBound(unsigned i) const {
            int_fast64_t bound = getUpperBound<uint64_t>(i);
            if (isUpperBoundStrict(i)) {
                STORM_LOG_THROW(bound > 0, storm::exceptions::InvalidPropertyException, "Cannot retrieve non-strict bound from strict zero-bound.");
                return bound - 1;
            } else {
                return bound;
            }
        }

        template <>
        double BoundedGloballyFormula::getNonStrictLowerBound(unsigned i) const {
            double bound = getLowerBound<double>(i);
            STORM_LOG_THROW(!isLowerBoundStrict(i), storm::exceptions::InvalidPropertyException, "Cannot retrieve non-strict lower bound from strict lower-bound.");
            return bound;
        }

        template <>
        uint64_t BoundedGloballyFormula::getNonStrictLowerBound(unsigned i) const {
            int_fast64_t bound = getLowerBound<uint64_t>(i);
            if (isLowerBoundStrict(i)) {
                return bound + 1;
            } else {
                return bound;
            }
        }

        void BoundedGloballyFormula::checkNoVariablesInBound(storm::expressions::Expression const& bound) {
            STORM_LOG_THROW(!bound.containsVariables(), storm::exceptions::InvalidOperationException, "Cannot evaluate time-bound '" << bound << "' as it contains undefined constants.");
        }

        std::ostream& BoundedGloballyFormula::writeToStream(std::ostream& out, bool allowParentheses) const {
            out << "G" ;
            if (this->isMultiDimensional()) {
                out << "^{";
            }
            for (unsigned i = 0; i < this->getDimension(); ++i) {
                if (i > 0) {
                    out << ", ";
                }
                if (this->getTimeBoundReference(i).isRewardBound()) {
                    out << "rew";
                    if (this->getTimeBoundReference(i).hasRewardAccumulation()) {
                        out << "[" << this->getTimeBoundReference(i).getRewardAccumulation() << "]";
                    }
                    out << "{\"" << this->getTimeBoundReference(i).getRewardName() << "\"}";
                } else if (this->getTimeBoundReference(i).isStepBound()) {
                    out << "steps";
                }
                if (this->hasLowerBound(i)) {
                    if (this->hasUpperBound(i)) {
                        if (this->isLowerBoundStrict(i)) {
                            out << "(";
                        } else {
                            out << "[";
                        }
                        out << this->getLowerBound(i);
                        out << ", ";
                        out << this->getUpperBound(i);
                        if (this->isUpperBoundStrict(i)) {
                            out << ")";
                        } else {
                            out << "]";
                        }
                    } else {
                        if (this->isLowerBoundStrict(i)) {
                            out << ">";
                        } else {
                            out << ">=";
                        }
                        out << getLowerBound(i);
                    }
                } else {
                    if (this->isUpperBoundStrict(i)) {
                        out << "<";
                    } else {
                        out << "<=";
                    }
                    out << this->getUpperBound(i);
                }
                out << " ";
            }
            if (this->isMultiDimensional()) {
                out << "}";
            }
            this->getSubformula().writeToStream(out);

            return out;
        }
    }
}
