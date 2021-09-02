#pragma once

#include <boost/optional.hpp>

#include "storm/logic/UnaryPathFormula.h"

#include "storm/logic/TimeBound.h"
#include "storm/logic/TimeBoundType.h"

namespace storm {
    namespace logic {
        class BoundedGloballyFormula : public UnaryPathFormula {
        public:
            BoundedGloballyFormula(std::shared_ptr<Formula const> const& subformula, boost::optional<TimeBound> const& lowerBound, boost::optional<TimeBound> const& upperBound, TimeBoundReference const& timeBoundReference);
            BoundedGloballyFormula(std::shared_ptr<Formula const> const& subformula, std::vector<boost::optional<TimeBound>> const& lowerBounds, std::vector<boost::optional<TimeBound>> const& upperBounds, std::vector<TimeBoundReference> const& timeBoundReferences);
            BoundedGloballyFormula(std::vector<std::shared_ptr<Formula const>> const& subformulas, std::vector<boost::optional<TimeBound>> const& lowerBounds, std::vector<boost::optional<TimeBound>> const& upperBounds, std::vector<TimeBoundReference> const& timeBoundReferences);

            virtual ~BoundedGloballyFormula() {
                // Intentionally left empty.
            }

            virtual bool isBoundedGloballyFormula() const override;
            virtual bool isProbabilityPathFormula() const override;

            virtual boost::any accept(FormulaVisitor const &visitor, boost::any const &data) const override;



            bool isMultiDimensional() const;
            bool hasMultiDimensionalSubformulas() const;
            unsigned getDimension() const;

            Formula const& getSubformula() const;
            Formula const& getSubformula(unsigned i) const;

            TimeBoundReference const& getTimeBoundReference(unsigned i = 0) const;

            bool isLowerBoundStrict(unsigned i = 0) const;
            bool hasLowerBound() const;
            bool hasLowerBound(unsigned i) const;
            bool hasIntegerLowerBound(unsigned i = 0) const;

            bool isUpperBoundStrict(unsigned i = 0) const;
            bool hasUpperBound() const;
            bool hasUpperBound(unsigned i) const;
            bool hasIntegerUpperBound(unsigned i = 0) const;

            storm::expressions::Expression const& getLowerBound(unsigned i = 0) const;
            storm::expressions::Expression const& getUpperBound(unsigned i = 0) const;

            template <typename ValueType>
            ValueType getLowerBound(unsigned i = 0) const;

            template <typename ValueType>
            ValueType getUpperBound(unsigned i = 0) const;

            template <typename ValueType>
            ValueType getNonStrictUpperBound(unsigned i = 0) const;

            template<typename ValueType>
            ValueType getNonStrictLowerBound(unsigned i = 0) const;

            virtual std::ostream &writeToStream(std::ostream &out, bool allowParentheses = false) const override;
        private:
            static void checkNoVariablesInBound(storm::expressions::Expression const& bound);

            std::vector<std::shared_ptr<Formula const>> subformula;
            std::vector<TimeBoundReference> timeBoundReference;
            std::vector<boost::optional<TimeBound>> lowerBound;
            std::vector<boost::optional<TimeBound>> upperBound;
        };
    }
}
