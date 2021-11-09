#include "storm/logic/ShieldExpression.h"

#include <iostream>

namespace storm {
    namespace logic {
        ShieldExpression::ShieldExpression() {}

        ShieldExpression::ShieldExpression(ShieldingType type, std::string filename) : type(type), filename(filename) {
            //Intentionally left empty
        }

        ShieldExpression::ShieldExpression(ShieldingType type, std::string filename, ShieldComparison comparison, double value) : type(type), filename(filename), comparison(comparison), value(value) {
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

        bool ShieldExpression::isOptimalShield() const {
            return type == storm::logic::ShieldingType::OptimalPre || type == storm::logic::ShieldingType::OptimalPost;
        }

        bool ShieldExpression::isOptimalPreShield() const {
            return type == storm::logic::ShieldingType::OptimalPre;
        }

        bool ShieldExpression::isOptimalPostShield() const {
            return type == storm::logic::ShieldingType::OptimalPost;
        }

        double ShieldExpression::getValue() const {
            return value;
        }

        std::string ShieldExpression::typeToString() const {
            switch(type) {
                case storm::logic::ShieldingType::PostSafety: return "Post";
                case storm::logic::ShieldingType::PreSafety:  return "Pre";
                case storm::logic::ShieldingType::OptimalPre:    return "OptimalPre";
                case storm::logic::ShieldingType::OptimalPost:    return "OptimalPost";
            }
        }

        std::string ShieldExpression::comparisonToString() const {
            switch(comparison) {
                case storm::logic::ShieldComparison::Absolute: return "gamma";
                case storm::logic::ShieldComparison::Relative: return "lambda";
            }
        }

        std::string ShieldExpression::toString() const {
            return "<" + typeToString() + ", " + comparisonToString() + "=" + std::to_string(value) + ">";
        }

        std::string ShieldExpression::prettify() const {
            std::string prettyString = "";
            std::string comparisonType = isRelative() ? "relative" : "absolute";
            switch(type) {
                case storm::logic::ShieldingType::PostSafety:  prettyString += "Post-Safety"; break;
                case storm::logic::ShieldingType::PreSafety:   prettyString += "Pre-Safety"; break;
                case storm::logic::ShieldingType::OptimalPre:  prettyString += "Optimal-Pre"; break;
                case storm::logic::ShieldingType::OptimalPost: prettyString += "Optimal-Post"; break;
            }
            prettyString += "-Shield ";
            prettyString += "with " + comparisonType + " comparison (" + comparisonToString() + " = " + std::to_string(value) + "):";
            return prettyString;
        }

        std::string ShieldExpression::getFilename() const {
            return filename;
        }

        std::ostream& operator<<(std::ostream& out, ShieldExpression const& shieldExpression) {
            out << shieldExpression.toString();
            return out;
        }
    }
}
