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
                class GameViHelper {
                public:
                    GameViHelper(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::BitVector statesOfCoalition);

                    void prepareSolversAndMultipliersReachability(const Environment& env);

                    void performValueIteration(Environment const& env, std::vector<ValueType>& x, std::vector<ValueType> b, storm::solver::OptimizationDirection const dir);

                    /*h
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
                     * Checks whether the curently computed value achieves the desired precision
                     */
                    bool checkConvergence(ValueType precision) const;

                    std::vector<ValueType>& xNew();
                    std::vector<ValueType> const& xNew() const;

                    std::vector<ValueType>& xOld();
                    std::vector<ValueType> const& xOld() const;
                    bool _x1IsCurrent;

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
                    std::vector<ValueType> _x1, _x2, _b;
                    std::unique_ptr<storm::solver::Multiplier<ValueType>> _multiplier;

                    bool _produceScheduler = false;
                    boost::optional<std::vector<uint64_t>> _producedOptimalChoices;
                };
            }
        }
    }
}