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

        void setOptimizationDirectionOverride(storm::storage::BitVector optimizationDirectionOverride);
        boost::optional<storm::storage::BitVector> const& getOptimizationDirectionOverride() const;

    private:
        storm::solver::MultiplierType type;
        bool typeSetFromDefault;

        boost::optional<storm::storage::BitVector> optimizationDirectionOverride = boost::none;
    };
}
