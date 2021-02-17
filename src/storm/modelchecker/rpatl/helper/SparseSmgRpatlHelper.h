#pragma once

#include <vector>

#include "storm/modelchecker/hints/ModelCheckerHint.h"
#include "storm/modelchecker/prctl/helper/SolutionType.h"
#include "storm/storage/SparseMatrix.h"

#include "storm/utility/solver.h"
#include "storm/solver/SolveGoal.h"

#include "storm/modelchecker/prctl/helper/MDPModelCheckingHelperReturnType.h"

namespace storm {

    class Environment;

    namespace storage {
        class BitVector;
    }

    namespace models {
        namespace sparse {
            template <typename ValueType>
            class StandardRewardModel;
        }
    }

    namespace modelchecker {
        class CheckResult;

        namespace helper {

            template <typename ValueType>
            class SparseSmgRpatlHelper {
            public:
                // TODO should probably be renamed in the future:

                static MDPSparseModelCheckingHelperReturnType<ValueType> computeUntilProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& phiStates, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint = ModelCheckerHint());

            private:
                static storm::storage::Scheduler<ValueType> expandScheduler(storm::storage::Scheduler<ValueType> scheduler, storm::storage::BitVector psiStates, storm::storage::BitVector notPhiStates);

            };
        }
    }
}
