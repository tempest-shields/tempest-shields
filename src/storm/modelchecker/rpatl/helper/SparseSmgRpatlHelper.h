#pragma once

#include <vector>

#include "storm/modelchecker/hints/ModelCheckerHint.h"
#include "storm/modelchecker/prctl/helper/SolutionType.h"
#include "storm/storage/SparseMatrix.h"

#include "storm/utility/solver.h"
#include "storm/solver/SolveGoal.h"

#include "storm/modelchecker/rpatl/helper/SMGModelCheckingHelperReturnType.h"

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
                static SMGSparseModelCheckingHelperReturnType<ValueType> computeUntilProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& phiStates, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint = ModelCheckerHint());
                static SMGSparseModelCheckingHelperReturnType<ValueType> computeGloballyProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint = ModelCheckerHint());
                static SMGSparseModelCheckingHelperReturnType<ValueType> computeNextProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint);
                static SMGSparseModelCheckingHelperReturnType<ValueType> computeBoundedGloballyProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint, uint64_t lowerBound, uint64_t upperBound);
                static SMGSparseModelCheckingHelperReturnType<ValueType> computeBoundedUntilProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& phiStates, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint, uint64_t lowerBound, uint64_t upperBound, bool computeBoundedGlobally = false);
            private:
                static storm::storage::Scheduler<ValueType> expandScheduler(storm::storage::Scheduler<ValueType> scheduler, storm::storage::BitVector psiStates, storm::storage::BitVector notPhiStates);
                static void expandChoiceValues(std::vector<uint_fast64_t> const& rowGroupIndices, storm::storage::BitVector const& relevantStates, std::vector<ValueType> const& constrainedChoiceValues, std::vector<ValueType>& choiceValues);
            };
        }
    }
}
