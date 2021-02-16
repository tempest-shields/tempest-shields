#include "storm/logic/ShieldExpression.h"

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
    }
}
