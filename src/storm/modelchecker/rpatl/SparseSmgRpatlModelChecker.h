#ifndef STORM_MODELCHECKER_SPARSESMGRPATLMODELCHECKER_H_
#define STORM_MODELCHECKER_SPARSESMGRPATLMODELCHECKER_H_


#include "storm/modelchecker/propositional/SparsePropositionalModelChecker.h"
#include "storm/models/sparse/Smg.h"
#include "storm/utility/solver.h"
#include "storm/solver/LinearEquationSolver.h"
#include "storm/storage/StronglyConnectedComponent.h"
#include "storm/storage/BitVector.h"

namespace storm {
    namespace modelchecker {

        template<class SparseSmgModelType>
        class SparseSmgRpatlModelChecker : public SparsePropositionalModelChecker<SparseSmgModelType> {
        public:
            typedef typename SparseSmgModelType::ValueType ValueType;
            typedef typename SparseSmgModelType::RewardModelType RewardModelType;

            explicit SparseSmgRpatlModelChecker(SparseSmgModelType const& model);

            /*!
             * Returns false, if this task can certainly not be handled by this model checker (independent of the concrete model).
             * @param requiresSingleInitialState if not nullptr, this flag is set to true iff checking this formula requires a model with a single initial state
             */
            static bool canHandleStatic(CheckTask<storm::logic::Formula, ValueType> const& checkTask, bool* requiresSingleInitialState = nullptr);

            // The implemented methods of the AbstractModelChecker interface.
            bool canHandle(CheckTask<storm::logic::Formula, ValueType> const& checkTask) const override;

            std::unique_ptr<CheckResult> checkGameFormula(Environment const& env, CheckTask<storm::logic::GameFormula, ValueType> const& checkTask) override;
            std::unique_ptr<CheckResult> checkProbabilityOperatorFormula(Environment const& env, CheckTask<storm::logic::ProbabilityOperatorFormula, ValueType> const& checkTask) override;
            std::unique_ptr<CheckResult> checkRewardOperatorFormula(Environment const& env, CheckTask<storm::logic::RewardOperatorFormula, ValueType> const& checkTask) override;
            std::unique_ptr<CheckResult> checkLongRunAverageOperatorFormula(Environment const& env, CheckTask<storm::logic::LongRunAverageOperatorFormula, ValueType> const& checkTask) override;

            std::unique_ptr<CheckResult> computeProbabilities(Environment const& env, CheckTask<storm::logic::Formula, ValueType> const& checkTask) override;
            std::unique_ptr<CheckResult> computeRewards(Environment const& env, storm::logic::RewardMeasureType rewardMeasureType, CheckTask<storm::logic::Formula, ValueType> const& checkTask) override;

            std::unique_ptr<CheckResult> computeUntilProbabilities(Environment const& env, CheckTask<storm::logic::UntilFormula, ValueType> const& checkTask) override;
            std::unique_ptr<CheckResult> computeGloballyProbabilities(Environment const& env, CheckTask<storm::logic::GloballyFormula, ValueType> const& checkTask) override;
            std::unique_ptr<CheckResult> computeNextProbabilities(Environment const& env, CheckTask<storm::logic::NextFormula, ValueType> const& checkTask) override;
            std::unique_ptr<CheckResult> computeBoundedGloballyProbabilities(Environment const& env, CheckTask<storm::logic::BoundedGloballyFormula, ValueType> const& checkTask) override;
            std::unique_ptr<CheckResult> computeBoundedUntilProbabilities(Environment const& env, CheckTask<storm::logic::BoundedUntilFormula, ValueType> const& checkTask) override;

            std::unique_ptr<CheckResult> computeLongRunAverageProbabilities(Environment const& env, CheckTask<storm::logic::StateFormula, ValueType> const& checkTask) override;
            std::unique_ptr<CheckResult> computeLongRunAverageRewards(Environment const& env, storm::logic::RewardMeasureType rewardMeasureType, CheckTask<storm::logic::LongRunAverageRewardFormula, ValueType> const& checkTask) override;

        private:
            storm::storage::BitVector statesOfCoalition;
        };
    } // namespace modelchecker
} // namespace storm

#endif /* STORM_MODELCHECKER_SPARSESMGRPATLMODELCHECKER_H_ */
