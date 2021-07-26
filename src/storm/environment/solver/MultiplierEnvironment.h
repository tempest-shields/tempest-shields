#pragma once

#include <boost/optional.hpp>

#include "storm/environment/solver/SolverEnvironment.h"
#include "storm/solver/SolverSelectionOptions.h"

#include "storm/storage/BitVector.h"

namespace storm {

    class MultiplierEnvironment {
    public:

        MultiplierEnvironment();
        ~MultiplierEnvironment();

        storm::solver::MultiplierType const& getType() const;
        bool const& isTypeSetFromDefault() const;
        void setType(storm::solver::MultiplierType value, bool isSetFromDefault = false);
    private:
        storm::solver::MultiplierType type;
        bool typeSetFromDefault;
    };
}
