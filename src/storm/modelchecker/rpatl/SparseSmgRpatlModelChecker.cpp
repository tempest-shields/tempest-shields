#include "storm/modelchecker/rpatl/SparseSmgRpatlModelChecker.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <boost/variant/get.hpp>

#include "storm/utility/macros.h"
#include "storm/utility/FilteredRewardModel.h"

#include "storm/modelchecker/results/ExplicitQualitativeCheckResult.h"
#include "storm/modelchecker/results/ExplicitQuantitativeCheckResult.h"
#include "storm/modelchecker/results/ExplicitParetoCurveCheckResult.h"

#include "storm/modelchecker/helper/infinitehorizon/SparseNondeterministicGameInfiniteHorizonHelper.h"
#include "storm/modelchecker/helper/utility/SetInformationFromCheckTask.h"

#include "storm/logic/FragmentSpecification.h"
#include "storm/logic/Coalition.h"

#include "storm/storage/BitVector.h"
#include "storm/environment/solver/MultiplierEnvironment.h"

#include "storm/models/sparse/StandardRewardModel.h"

#include "storm/settings/modules/GeneralSettings.h"

#include "storm/exceptions/InvalidStateException.h"
#include "storm/exceptions/InvalidPropertyException.h"
#include "storm/exceptions/InvalidArgumentException.h"

namespace storm {
    namespace modelchecker {
        template<typename SparseSmgModelType>
        SparseSmgRpatlModelChecker<SparseSmgModelType>::SparseSmgRpatlModelChecker(SparseSmgModelType const& model) : SparsePropositionalModelChecker<SparseSmgModelType>(model) {
            // Intentionally left empty.
        }



        template<typename SparseSmgModelType>
        bool SparseSmgRpatlModelChecker<SparseSmgModelType>::canHandleStatic(CheckTask<storm::logic::Formula, ValueType> const& checkTask, bool* requiresSingleInitialState) {
            storm::logic::Formula const& formula = checkTask.getFormula();
            if (formula.isInFragment(storm::logic::rpatl().setCoalitionOperatorsAllowed(true).setRewardOperatorsAllowed(true).setLongRunAverageRewardFormulasAllowed(true).setLongRunAverageProbabilitiesAllowed(true).setLongRunAverageOperatorsAllowed(true))) {
                return true;
            } else {
                return false;
            }
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
            Environment solverEnv = env;
            coalitionIndicator(solverEnv, checkTask);


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
            STORM_LOG_THROW(false, storm::exceptions::NotImplementedException, "NYI");
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



        template<typename SparseSmgModelType>
        void SparseSmgRpatlModelChecker<SparseSmgModelType>::coalitionIndicator(Environment& env, CheckTask<storm::logic::GameFormula, ValueType> const& checkTask) {
            storm::storage::BitVector coalitionIndicators(this->getModel().getTransitionMatrix().getRowGroupCount());

            std::vector<boost::variant<std::string, uint_fast64_t>> formulaPlayerIds = checkTask.getFormula().getCoalition().getPlayerIds();
            std::vector<uint_fast64_t> playerIds;
            std::vector<std::pair<std::string, uint_fast64_t>> playerActionIndices = this->getModel().getPlayerActionIndices();

            for(auto const& player : formulaPlayerIds) {
                // If the player is given via the player name we have to look up its index
                if(player.type() == typeid(std::string)) {
                    auto it = std::find_if(playerActionIndices.begin(), playerActionIndices.end(),
                                           [&player](const std::pair<std::string, uint_fast64_t>& element){ return element.first == boost::get<std::string>(player); });
                    playerIds.push_back(it->second);
                // If the player is given by its index we have to shift it to match internal mappings
                } else if(player.type() == typeid(uint_fast64_t)) {
                    playerIds.push_back(boost::get<uint_fast64_t>(player) - 1);
                }
            }
            //for(auto const& p : playerActionIndices) std::cout << p.first << " - " << p.second << ", "; std::cout << std::endl;
            //for(auto const& p : playerIds) std::cout << p << ", "; std::cout << std::endl;

            for(uint i = 0; i < playerActionIndices.size(); i++) {
                if(std::find(playerIds.begin(), playerIds.end(), playerActionIndices.at(i).second) != playerIds.end()) {
                    coalitionIndicators.set(i);
                }
            }
            coalitionIndicators.complement();

            //std::cout << "MINMAX OVERRIDE: " << coalitionIndicators << std::endl;
            env.solver().multiplier().setOptimizationDirectionOverride(coalitionIndicators);
        }

        template class SparseSmgRpatlModelChecker<storm::models::sparse::Smg<double>>;
#ifdef STORM_HAVE_CARL
        template class SparseSmgRpatlModelChecker<storm::models::sparse::Smg<storm::RationalNumber>>;
#endif
    }
}
