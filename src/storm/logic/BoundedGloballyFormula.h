#ifndef STORM_BOUNDEDGLOBALLYFORMULA_H
#define STORM_BOUNDEDGLOBALLYFORMULA_H

#include "storm/logic/UnaryPathFormula.h"

namespace storm {
    namespace logic {
        class BoundedGloballyFormula : public UnaryPathFormula {
        public:
            BoundedGloballyFormula(std::shared_ptr<Formula const> const &subformula);

            virtual ~BoundedGloballyFormula() {
                // Intentionally left empty.
            }

            virtual bool isBoundedGloballyFormula() const override;
            virtual bool isProbabilityPathFormula() const override;

            virtual boost::any accept(FormulaVisitor const &visitor, boost::any const &data) const override;
            virtual std::ostream &writeToStream(std::ostream &out) const override;
        };
    }
}

#endif //STORM_BOUNDEDGLOBALLYFORMULA_H
