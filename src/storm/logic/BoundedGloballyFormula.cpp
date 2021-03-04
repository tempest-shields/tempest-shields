#include "storm/logic/BoundedGloballyFormula.h"
#include "storm/logic/FormulaVisitor.h"

namespace storm {
    namespace logic {
        BoundedGloballyFormula::BoundedGloballyFormula(std::shared_ptr<Formula const> const& subformula) : UnaryPathFormula(subformula) {
            // Intentionally left empty.
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

        std::ostream& BoundedGloballyFormula::writeToStream(std::ostream& out) const {
            out << "G < ";
            this->getSubformula().writeToStream(out);
            return out;
        }
    }
}