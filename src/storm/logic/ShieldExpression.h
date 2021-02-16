#pragma once

#include <memory>

namespace storm {
    namespace logic {

        enum class ShieldingType {
            PostSafety,
            PreSafety,
            Optimal
        };

        enum class ShieldComparison { Absolute, Relative };

        class ShieldExpression {
        public:
            ShieldExpression();
            ShieldExpression(ShieldingType type, ShieldComparison comparison, double value);

            bool isRelative() const;
            bool isPreSafetyShield() const;
            bool isPostSafetyShield() const;
            bool isOptiomalShield() const;

            double getValue() const;

        private:
            ShieldingType type;
            ShieldComparison comparison;
            double value;
        };
    }
}
