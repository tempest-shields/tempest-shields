#include "storm/modelchecker/rpatl/SparseSmgRpatlModelChecker.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <boost/variant/get.hpp>

#include "storm/utility/macros.h"
#include "storm/utility/FilteredRewardModel.h"

#include "storm/modelchecker/results/ExplicitQualitativeCheckResult.h"
#include "storm/modelchecker/results/ExplicitQuantitativeCheckResult.h"

#include "storm/modelchecker/helper/utility/SetInformationFromCheckTask.h"

#include "storm/logic/FragmentSpecification.h"
#include "storm/logic/Coalition.h"

#include "storm/storage/BitVector.h"
#include "storm/environment/solver/MultiplierEnvironment.h"

#include "storm/models/sparse/StandardRewardModel.h"

#include "storm/settings/modules/GeneralSettings.h"

#include "storm/exceptions/InvalidPropertyException.h"
#include "storm/exceptions/NotImplementedException.h"

namespace storm {
    namespace modelchecker {
        template<typename SparseSmgModelType>
        SparseSmgRpatlModelChecker<SparseSmgModelType>::SparseSmgRpatlModelChecker(SparseSmgModelType const& model) : SparsePropositionalModelChecker<SparseSmgModelType>(model) {
            // Intentionally left empty.
        }



        template<typename SparseSmgModelType>
        bool SparseSmgRpatlModelChecker<SparseSmgModelType>::canHandleStatic(CheckTask<storm::logic::Formula, ValueType> const& checkTask, bool* requiresSingleInitialState) {
            storm::logic::Formula const& formula = checkTask.getFormula();

            return formula.isInFragment(storm::logic::rpatl());
        }

        template<typename SparseSmgModelType>
        bool SparseSmgRpatlModelChecker<SparseSmgModelType>::canHandle(CheckTask<storm::logic::Formula, ValueType> const& checkTask) const {
            bool requiresSingleInitialState = false;
            if (canHandleStatic(checkTask, &requiresSingleInitialState)) {
                return !requiresSingleInitialState || this->getModel().getInitialStates().getNumberOfSetBits() == 1;
            } else {
                return false;
            }
        }



        template<typename SparseSmgModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<SparseSmgModelType>::checkGameFormula(Environment const& env, CheckTask<storm::logic::GameFormula, ValueType> const& checkTask) {
            storm::logic::GameFormula const& gameFormula = checkTask.getFormula();
            storm::logic::Formula const& subFormula = gameFormula.getSubformula();

            if (subFormula.isRewardOperatorFormula()) {
                return this->checkRewardOperatorFormula(solverEnv, checkTask.substituteFormula(subFormula.asRewardOperatorFormula()));
            } else if (subFormula.isLongRunAverageOperatorFormula()) {
                return this->checkLongRunAverageOperatorFormula(solverEnv, checkTask.substituteFormula(subFormula.asLongRunAverageOperatorFormula()));
            }
            STORM_LOG_THROW(false, storm::exceptions::NotImplementedException, "Cannot check this property (yet).");
        }

        template<typename ModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<ModelType>::checkRewardOperatorFormula(Environment const& env, CheckTask<storm::logic::RewardOperatorFormula, ValueType> const& checkTask) {
            storm::logic::RewardOperatorFormula const& formula = checkTask.getFormula();
            std::unique_ptr<CheckResult> result = this->computeRewards(env, formula.getMeasureType(), checkTask.substituteFormula(formula.getSubformula()));

            return result;
        }

        template<typename ModelType>
		std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<ModelType>::checkLongRunAverageOperatorFormula(Environment const& env, CheckTask<storm::logic::LongRunAverageOperatorFormula, ValueType> const& checkTask) {
            storm::logic::LongRunAverageOperatorFormula const& formula = checkTask.getFormula();
            std::unique_ptr<CheckResult> result = this->computeLongRunAverageProbabilities(env, checkTask.substituteFormula(formula.getSubformula().asStateFormula()));

            return result;
        }



        template<typename ModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<ModelType>::computeRewards(Environment const& env, storm::logic::RewardMeasureType rewardMeasureType, CheckTask<storm::logic::Formula, ValueType> const& checkTask) {
            storm::logic::Formula const& rewardFormula = checkTask.getFormula();
            if (rewardFormula.isLongRunAverageRewardFormula()) {
                return this->computeLongRunAverageRewards(env, rewardMeasureType, checkTask.substituteFormula(rewardFormula.asLongRunAverageRewardFormula()));
            }
            STORM_LOG_THROW(false, storm::exceptions::InvalidArgumentException, "The given formula '" << rewardFormula << "' cannot (yet) be handled.");
        }

        template<typename SparseSmgModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<SparseSmgModelType>::computeLongRunAverageProbabilities(Environment const& env, CheckTask<storm::logic::StateFormula, ValueType> const& checkTask) {
            STORM_LOG_THROW(false, storm::exceptions::NotImplementedException, "Not implemented.");
        }

        template<typename SparseSmgModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<SparseSmgModelType>::computeLongRunAverageRewards(Environment const& env, storm::logic::RewardMeasureType rewardMeasureType, CheckTask<storm::logic::LongRunAverageRewardFormula, ValueType> const& checkTask) {
            auto rewardModel = storm::utility::createFilteredRewardModel(this->getModel(), checkTask);
            storm::modelchecker::helper::SparseNondeterministicGameInfiniteHorizonHelper<ValueType> helper(this->getModel().getTransitionMatrix(), this->getModel().getPlayerActionIndices());
            storm::modelchecker::helper::setInformationFromCheckTaskNondeterministic(helper, checkTask, this->getModel());
			auto values = helper.computeLongRunAverageRewards(env, rewardModel.get());

            std::unique_ptr<CheckResult> result(new ExplicitQuantitativeCheckResult<ValueType>(std::move(values)));
            if (checkTask.isProduceSchedulersSet()) {
                result->asExplicitQuantitativeCheckResult<ValueType>().setScheduler(std::make_unique<storm::storage::Scheduler<ValueType>>(helper.extractScheduler()));
            }
            return result;
        }

        template class SparseSmgRpatlModelChecker<storm::models::sparse::Smg<double>>;
#ifdef STORM_HAVE_CARL
        template class SparseSmgRpatlModelChecker<storm::models::sparse::Smg<storm::RationalNumber>>;
#endif
    }
}
