#pragma once


#include "storm/modelchecker/hints/ModelCheckerHint.h"
#include "storm/modelchecker/prctl/helper/SolutionType.h"
#include "storm/storage/SparseMatrix.h"
#include "storm/storage/BitVector.h"
#include "storm/utility/solver.h"
#include "storm/solver/SolveGoal.h"

namespace storm {
    namespace modelchecker {
        namespace helper {

            template<typename ValueType>
            class SparseNondeterministicStepBoundedHorizonHelper {
            public:
                SparseNondeterministicStepBoundedHorizonHelper();

                void getMaybeStatesRowGroupSizes(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::BitVector const& maybeStates, std::vector<uint64_t>& maybeStatesRowGroupSizes, uint& choiceValuesCounter);
                void getChoiceValues(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::BitVector const& maybeStates, std::vector<ValueType> const& choiceValues, std::vector<ValueType>& allChoices);
                std::vector<ValueType> compute(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& phiStates, storm::storage::BitVector const& psiStates, uint64_t lowerBound, uint64_t upperBound, ModelCheckerHint const& hint = ModelCheckerHint(), storm::storage::BitVector& resultMaybeStates = nullptr, std::vector<ValueType>& choiceValues = nullptr);
            private:
            };


        }
    }
}
