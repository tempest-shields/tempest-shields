#pragma once
#include "storm/modelchecker/helper/infinitehorizon/SparseInfiniteHorizonHelper.h"

namespace storm {

    namespace storage {
        template <typename VT> class Scheduler;
    }

    namespace modelchecker {
        namespace helper {

            /*!
             * Helper class for model checking queries that depend on the long run behavior of the (nondeterministic) system with different players choices.
             * @tparam ValueType the type a value can have
             */
            template <typename ValueType>
            class SparseNondeterministicGameInfiniteHorizonHelper : public SparseInfiniteHorizonHelper<ValueType, true> {

            public:

                /*!
                 * Function mapping from indices to values
                 */
                typedef typename SparseInfiniteHorizonHelper<ValueType, true>::ValueGetter ValueGetter;

                /*!
                 * Initializes the helper for a discrete time model with different players (i.e. SMG)
                 */
                SparseNondeterministicGameInfiniteHorizonHelper(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, std::vector<uint64_t> const& coalitionIndices);

                /*!
                 * TODO
                 */
                std::vector<ValueType> computeLongRunAverageValues(Environment const& env, ValueGetter const& stateRewardsGetter, ValueGetter const& actionRewardsGetter);

                /*!
                 * @pre before calling this, a computation call should have been performed during which scheduler production was enabled.
                 * @return the produced scheduler of the most recent call.
                 */
                //std::vector<uint64_t> const& getProducedOptimalChoices() const;

                /*!
                 * @pre before calling this, a computation call should have been performed during which scheduler production was enabled.
                 * @return the produced scheduler of the most recent call.
                 */
                //std::vector<uint64_t>& getProducedOptimalChoices();

                /*!
                 * @pre before calling this, a computation call should have been performed during which scheduler production was enabled.
                 * @return a new scheduler containing optimal choices for each state that yield the long run average values of the most recent call.
                 */
                //storm::storage::Scheduler<ValueType> extractScheduler() const;

                void createDecomposition();
                ValueType computeLraForComponent(Environment const& env, ValueGetter const& stateValuesGetter,  ValueGetter const& actionValuesGetter, storm::storage::MaximalEndComponent const& component);
                std::vector<ValueType> buildAndSolveSsp(Environment const& env, std::vector<ValueType> const& mecLraValues);

            private:
                std::vector<uint64_t> coalitionIndices;
            };


        }
    }
}
