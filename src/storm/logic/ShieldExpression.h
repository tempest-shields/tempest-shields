#pragma once

#include <memory>
#include <string>

namespace storm {
    namespace logic {

        enum class ShieldingType {
            PostSafety,
            PreSafety,
            OptimalPre,
            OptimalPost
        };

        enum class ShieldComparison { Absolute, Relative };

        class ShieldExpression {
        public:
            ShieldExpression();
            ShieldExpression(ShieldingType type, std::string filename);
            ShieldExpression(ShieldingType type, std::string filename, ShieldComparison comparison, double value);

            bool isRelative() const;
            bool isPreSafetyShield() const;
            bool isPostSafetyShield() const;
            bool isOptimalShield() const;
            bool isOptimalPreShield() const;
            bool isOptimalPostShield() const;

            double getValue() const;

            std::string typeToString() const;
            std::string comparisonToString() const;
            std::string toString() const;
            std::string prettify() const;
            std::string getFilename() const;
            friend std::ostream& operator<<(std::ostream& stream, ShieldExpression const& shieldExpression);

        private:
            ShieldingType type;
            ShieldComparison comparison = ShieldComparison::Relative;
            double value = 0;

            std::string filename;
        };
    }
}
