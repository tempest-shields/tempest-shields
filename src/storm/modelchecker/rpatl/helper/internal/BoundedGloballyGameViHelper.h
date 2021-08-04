#pragma once

#include "storm/storage/SparseMatrix.h"
#include "storm/solver/LinearEquationSolver.h"
#include "storm/solver/MinMaxLinearEquationSolver.h"
#include "storm/solver/Multiplier.h"

namespace storm {
    class Environment;

    namespace storage {
        template <typename VT> class Scheduler;
    }

    namespace modelchecker {
        namespace helper {
            namespace internal {

                template <typename ValueType>
                class BoundedGloballyGameViHelper {
                public:
                    BoundedGloballyGameViHelper(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::BitVector statesOfCoalition);

                    void prepareSolversAndMultipliers(const Environment& env);

                    void performValueIteration(Environment const& env, std::vector<ValueType>& x, storm::solver::OptimizationDirection const dir, uint64_t upperBound, std::vector<ValueType>& constrainedChoiceValues);

                    /*!
                     * Fills the result vector to the original size with zeros for all states except the relevantStates
                     */
                    void fillResultVector(std::vector<ValueType>& result, storm::storage::BitVector relevantStates);

                    /*!
                     * Fills the choice values vector to the original size with zeros for ~psiState choices.
                     */
                    void fillChoiceValuesVector(std::vector<ValueType>& choiceValues, storm::storage::BitVector psiStates, std::vector<storm::storage::SparseMatrix<double>::index_type> rowGroupIndices);

                    /*!
                     * Sets whether an optimal scheduler shall be constructed during the computation
                     */
                    void setProduceScheduler(bool value);

                    /*!
                     * @return whether an optimal scheduler shall be constructed during the computation
                     */
                    bool isProduceSchedulerSet() const;

                    storm::storage::Scheduler<ValueType> extractScheduler() const;

                private:
                    void performIterationStep(Environment const& env, storm::solver::OptimizationDirection const dir, std::vector<uint64_t>* choices = nullptr);

                    /*!
                     * @pre before calling this, a computation call should have been performed during which scheduler production was enabled.
                     * @return the produced scheduler of the most recent call.
                     */
                    std::vector<uint64_t> const& getProducedOptimalChoices() const;

                    /*!
                     * @pre before calling this, a computation call should have been performed during which scheduler production was enabled.
                     * @return the produced scheduler of the most recent call.
                     */
                    std::vector<uint64_t>& getProducedOptimalChoices();

                    storm::storage::SparseMatrix<ValueType> _transitionMatrix;
                    storm::storage::BitVector _statesOfCoalition;
                    std::vector<ValueType> _x;
                    std::unique_ptr<storm::solver::Multiplier<ValueType>> _multiplier;

                    bool _produceScheduler = false;
                    boost::optional<std::vector<uint64_t>> _producedOptimalChoices;
                };
            }
        }
    }
}
