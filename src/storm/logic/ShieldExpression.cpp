#include "storm/logic/ShieldExpression.h"

#include <iostream>

namespace storm {
    namespace logic {
        ShieldExpression::ShieldExpression() {}

        ShieldExpression::ShieldExpression(ShieldingType type, ShieldComparison comparison, double value) : type(type), comparison(comparison), value(value) {
            //Intentionally left empty
        }

        bool ShieldExpression::isRelative() const {
            return comparison == storm::logic::ShieldComparison::Relative;
        }

        bool ShieldExpression::isPreSafetyShield() const {
            return type == storm::logic::ShieldingType::PreSafety;
        }

        bool ShieldExpression::isPostSafetyShield() const {
            return type == storm::logic::ShieldingType::PostSafety;
        }

        bool ShieldExpression::isOptiomalShield() const {
            return type == storm::logic::ShieldingType::Optimal;
        }

        double ShieldExpression::getValue() const {
            return value;
        }

        std::string ShieldExpression::typeToString() const {
            switch(type) {
                case storm::logic::ShieldingType::PostSafety: return "PostSafety";
                case storm::logic::ShieldingType::PreSafety:  return "PreSafety";
                case storm::logic::ShieldingType::Optimal:    return "Optimal";
            }
        }

        std::string ShieldExpression::comparisonToString() const {
            switch(comparison) {
                case storm::logic::ShieldComparison::Absolute: return "gamma";
                case storm::logic::ShieldComparison::Relative: return "lambda";
            }
        }

        std::ostream& operator<<(std::ostream& out, ShieldExpression const& shieldExpression) {
            out << "<" << shieldExpression.typeToString() << ", " << shieldExpression.comparisonToString() << "=" << std::to_string(shieldExpression.value) << ">";
            return out;
        }
    }
}
