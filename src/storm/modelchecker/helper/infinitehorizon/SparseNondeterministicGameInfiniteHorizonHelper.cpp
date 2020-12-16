#include "SparseNondeterministicGameInfiniteHorizonHelper.h"

#include "storm/modelchecker/helper/infinitehorizon/internal/LraViHelper.h"

#include "storm/storage/SparseMatrix.h"
#include "storm/storage/Scheduler.h"

#include "storm/solver/MinMaxLinearEquationSolver.h"
#include "storm/solver/Multiplier.h"

#include "storm/utility/solver.h"
#include "storm/utility/vector.h"

#include "storm/environment/solver/LongRunAverageSolverEnvironment.h"
#include "storm/environment/solver/MinMaxSolverEnvironment.h"

#include "storm/exceptions/UnmetRequirementException.h"
#include "storm/exceptions/InternalException.h"

namespace storm {
    namespace modelchecker {
        namespace helper {

            template <typename ValueType>
            SparseNondeterministicGameInfiniteHorizonHelper<ValueType>::SparseNondeterministicGameInfiniteHorizonHelper(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, std::vector<uint64_t> const& coalitionIndices) : SparseInfiniteHorizonHelper<ValueType, true>(transitionMatrix), coalitionIndices(coalitionIndices) {
                // Intentionally left empty.
            }

            template <typename ValueType>
            void SparseNondeterministicGameInfiniteHorizonHelper<ValueType>::createDecomposition() {
                STORM_LOG_THROW(false, storm::exceptions::InternalException, "Creating Decompositions of SMGs is currently not possible.");
            }

            template <typename ValueType>
            ValueType SparseNondeterministicGameInfiniteHorizonHelper<ValueType>::computeLraForComponent(Environment const& env, ValueGetter const& stateRewardsGetter, ValueGetter const& actionRewardsGetter, storm::storage::MaximalEndComponent const& component) {

                STORM_LOG_THROW(false, storm::exceptions::InternalException, "Computing values for LRA for SMGs components is currently not possible.");
            }

            template <typename ValueType>
            std::vector<ValueType> SparseNondeterministicGameInfiniteHorizonHelper<ValueType>::buildAndSolveSsp(Environment const& env, std::vector<ValueType> const& componentLraValues) {
                STORM_LOG_THROW(false, storm::exceptions::InternalException, "buildAndSolveSsp not available for SMGs");
            }

            template <typename ValueType>
            std::vector<ValueType> SparseNondeterministicGameInfiniteHorizonHelper<ValueType>::computeLongRunAverageValues(Environment const& env, ValueGetter const& stateRewardsGetter, ValueGetter const& actionRewardsGetter) {
                STORM_LOG_THROW(false, storm::exceptions::InternalException, "computeLongRunAverageValues not possible yet.");
            }


            template class SparseNondeterministicGameInfiniteHorizonHelper<double>;
            template class SparseNondeterministicGameInfiniteHorizonHelper<storm::RationalNumber>;

        }
    }
}
