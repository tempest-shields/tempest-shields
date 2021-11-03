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

#include "storm/modelchecker/rpatl/helper/SparseSmgRpatlHelper.h"
#include "storm/modelchecker/helper/infinitehorizon/SparseNondeterministicGameInfiniteHorizonHelper.h"
#include "storm/modelchecker/helper/utility/SetInformationFromCheckTask.h"

#include "storm/logic/FragmentSpecification.h"
#include "storm/logic/PlayerCoalition.h"

#include "storm/storage/BitVector.h"
#include "storm/environment/solver/MultiplierEnvironment.h"

#include "storm/models/sparse/StandardRewardModel.h"

#include "storm/shields/ShieldHandling.h"

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
            Environment solverEnv = env;

            storm::logic::GameFormula const& gameFormula = checkTask.getFormula();
            storm::logic::Formula const& subFormula = gameFormula.getSubformula();

            statesOfCoalition = this->getModel().computeStatesOfCoalition(gameFormula.getCoalition());
            STORM_LOG_INFO("Found " << statesOfCoalition.getNumberOfSetBits() << " states in coalition.");
            statesOfCoalition.complement();

            if (subFormula.isRewardOperatorFormula()) {
                return this->checkRewardOperatorFormula(solverEnv, checkTask.substituteFormula(subFormula.asRewardOperatorFormula()));
            } else if (subFormula.isLongRunAverageOperatorFormula()) {
                return this->checkLongRunAverageOperatorFormula(solverEnv, checkTask.substituteFormula(subFormula.asLongRunAverageOperatorFormula()));
            } else if (subFormula.isProbabilityOperatorFormula()) {
                return this->checkProbabilityOperatorFormula(solverEnv, checkTask.substituteFormula(subFormula.asProbabilityOperatorFormula()));
            }
            STORM_LOG_THROW(false, storm::exceptions::NotImplementedException, "Cannot check this property (yet).");
        }

        template<typename ModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<ModelType>::checkProbabilityOperatorFormula(Environment const& env, CheckTask<storm::logic::ProbabilityOperatorFormula, ValueType> const& checkTask) {
            storm::logic::ProbabilityOperatorFormula const& stateFormula = checkTask.getFormula();
            std::unique_ptr<CheckResult> result = this->computeProbabilities(env, checkTask.substituteFormula(stateFormula.getSubformula()));

            if (checkTask.isBoundSet()) {
                STORM_LOG_THROW(result->isQuantitative(), storm::exceptions::InvalidOperationException, "Unable to perform comparison operation on non-quantitative result.");
                return result->asQuantitativeCheckResult<ValueType>().compareAgainstBound(checkTask.getBoundComparisonType(), checkTask.getBoundThreshold());
            } else {
                return result;
            }
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
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<ModelType>::computeProbabilities(Environment const& env, CheckTask<storm::logic::Formula, ValueType> const& checkTask) {
            storm::logic::Formula const& formula = checkTask.getFormula();
            if (formula.isReachabilityProbabilityFormula()) {
                return this->computeReachabilityProbabilities(env, checkTask.substituteFormula(formula.asReachabilityProbabilityFormula()));
            } else if (formula.isUntilFormula()) {
                return this->computeUntilProbabilities(env, checkTask.substituteFormula(formula.asUntilFormula()));
            } else if (formula.isGloballyFormula()) {
                return this->computeGloballyProbabilities(env, checkTask.substituteFormula(formula.asGloballyFormula()));
            } else if (formula.isNextFormula()) {
                return this->computeNextProbabilities(env, checkTask.substituteFormula(formula.asNextFormula()));
            } else if (formula.isBoundedGloballyFormula()) {
                return this->computeBoundedGloballyProbabilities(env, checkTask.substituteFormula(formula.asBoundedGloballyFormula()));
            } else if (formula.isBoundedUntilFormula()) {
                return this->computeBoundedUntilProbabilities(env, checkTask.substituteFormula(formula.asBoundedUntilFormula()));
            }
            STORM_LOG_THROW(false, storm::exceptions::InvalidArgumentException, "The given formula '" << formula << "' is invalid.");
        }

        template<typename ModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<ModelType>::computeRewards(Environment const& env, storm::logic::RewardMeasureType rewardMeasureType, CheckTask<storm::logic::Formula, ValueType> const& checkTask) {
            storm::logic::Formula const& rewardFormula = checkTask.getFormula();
            if (rewardFormula.isLongRunAverageRewardFormula()) {
                return this->computeLongRunAverageRewards(env, rewardMeasureType, checkTask.substituteFormula(rewardFormula.asLongRunAverageRewardFormula()));
            }
            STORM_LOG_THROW(false, storm::exceptions::InvalidArgumentException, "The given formula '" << rewardFormula << "' cannot (yet) be handled.");
        }

        template<typename ModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<ModelType>::computeUntilProbabilities(Environment const& env, CheckTask<storm::logic::UntilFormula, ValueType> const& checkTask) {
            // Currently we only support computation of reachability probabilities, i.e. the left subformula will always be 'true' (for now).
            storm::logic::UntilFormula const& pathFormula = checkTask.getFormula();
            STORM_LOG_THROW(checkTask.isOptimizationDirectionSet(), storm::exceptions::InvalidPropertyException, "Formula needs to specify whether minimal or maximal values are to be computed on nondeterministic model.");
            std::unique_ptr<CheckResult> leftResultPointer = this->check(env, pathFormula.getLeftSubformula());
            std::unique_ptr<CheckResult> rightResultPointer = this->check(env, pathFormula.getRightSubformula());
            ExplicitQualitativeCheckResult const& leftResult = leftResultPointer->asExplicitQualitativeCheckResult();
            ExplicitQualitativeCheckResult const& rightResult = rightResultPointer->asExplicitQualitativeCheckResult();

            auto ret = storm::modelchecker::helper::SparseSmgRpatlHelper<ValueType>::computeUntilProbabilities(env, storm::solver::SolveGoal<ValueType>(this->getModel(), checkTask), this->getModel().getTransitionMatrix(), this->getModel().getBackwardTransitions(), leftResult.getTruthValuesVector(), rightResult.getTruthValuesVector(), checkTask.isQualitativeSet(), statesOfCoalition, checkTask.isProduceSchedulersSet(), checkTask.getHint());
            std::unique_ptr<CheckResult> result(new ExplicitQuantitativeCheckResult<ValueType>(std::move(ret.values)));
            if(checkTask.isShieldingTask()) {
                storm::storage::BitVector allStatesBv = storm::storage::BitVector(this->getModel().getTransitionMatrix().getRowGroupCount(), true);
                tempest::shields::createShield<ValueType>(std::make_shared<storm::models::sparse::Smg<ValueType>>(this->getModel()), std::move(ret.choiceValues), checkTask.getShieldingExpression(), checkTask.getOptimizationDirection(), allStatesBv, ~statesOfCoalition);
            } else if (checkTask.isProduceSchedulersSet() && ret.scheduler) {
                result->asExplicitQuantitativeCheckResult<ValueType>().setScheduler(std::move(ret.scheduler));
            }
            return result;
        }

        template<typename ModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<ModelType>::computeGloballyProbabilities(Environment const& env, CheckTask<storm::logic::GloballyFormula, ValueType> const& checkTask) {
            storm::logic::GloballyFormula const& pathFormula = checkTask.getFormula();
            std::unique_ptr<CheckResult> subResultPointer = this->check(env, pathFormula.getSubformula());
            ExplicitQualitativeCheckResult const& subResult = subResultPointer->asExplicitQualitativeCheckResult();

            auto ret = storm::modelchecker::helper::SparseSmgRpatlHelper<ValueType>::computeGloballyProbabilities(env, storm::solver::SolveGoal<ValueType>(this->getModel(), checkTask), this->getModel().getTransitionMatrix(), this->getModel().getBackwardTransitions(), subResult.getTruthValuesVector(), checkTask.isQualitativeSet(), statesOfCoalition, checkTask.isProduceSchedulersSet(), checkTask.getHint());
            std::unique_ptr<CheckResult> result(new ExplicitQuantitativeCheckResult<ValueType>(std::move(ret.values)));
            if(checkTask.isShieldingTask()) {
                storm::storage::BitVector allStatesBv = storm::storage::BitVector(this->getModel().getTransitionMatrix().getRowGroupCount(), true);
                tempest::shields::createShield<ValueType>(std::make_shared<storm::models::sparse::Smg<ValueType>>(this->getModel()), std::move(ret.choiceValues), checkTask.getShieldingExpression(), checkTask.getOptimizationDirection(), allStatesBv, ~statesOfCoalition);
            } else if (checkTask.isProduceSchedulersSet() && ret.scheduler) {
                result->asExplicitQuantitativeCheckResult<ValueType>().setScheduler(std::move(ret.scheduler));
            }
            return result;
        }

        template<typename ModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<ModelType>::computeNextProbabilities(Environment const& env, CheckTask<storm::logic::NextFormula, ValueType> const& checkTask) {
            storm::logic::NextFormula const& pathFormula = checkTask.getFormula();
            STORM_LOG_THROW(checkTask.isOptimizationDirectionSet(), storm::exceptions::InvalidPropertyException, "Formula needs to specify whether minimal or maximal values are to be computed on nondeterministic model.");
            std::unique_ptr<CheckResult> subResultPointer = this->check(env, pathFormula.getSubformula());
            ExplicitQualitativeCheckResult const& subResult = subResultPointer->asExplicitQualitativeCheckResult();

            auto ret = storm::modelchecker::helper::SparseSmgRpatlHelper<ValueType>::computeNextProbabilities(env, storm::solver::SolveGoal<ValueType>(this->getModel(), checkTask), this->getModel().getTransitionMatrix(), this->getModel().getBackwardTransitions(), subResult.getTruthValuesVector(), checkTask.isQualitativeSet(), ~statesOfCoalition, checkTask.isProduceSchedulersSet(), checkTask.getHint());
            std::unique_ptr<CheckResult> result(new ExplicitQuantitativeCheckResult<ValueType>(std::move(ret.values)));
            return result;
        }

        template<typename ModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<ModelType>::computeBoundedGloballyProbabilities(Environment const& env, CheckTask<storm::logic::BoundedGloballyFormula, ValueType> const& checkTask) {
            storm::logic::BoundedGloballyFormula const& pathFormula = checkTask.getFormula();
            STORM_LOG_THROW(checkTask.isOptimizationDirectionSet(), storm::exceptions::InvalidPropertyException, "Formula needs to specify whether minimal or maximal values are to be computed on nondeterministic model.");
            // checks for bounds
            STORM_LOG_THROW(pathFormula.hasUpperBound(), storm::exceptions::InvalidPropertyException, "Formula needs to have (a single) upper step bound.");
            STORM_LOG_THROW(pathFormula.hasIntegerLowerBound(), storm::exceptions::InvalidPropertyException, "Formula lower step bound must be discrete.");
            STORM_LOG_THROW(pathFormula.hasIntegerUpperBound(), storm::exceptions::InvalidPropertyException, "Formula upper step bound must be discrete.");

            std::unique_ptr<CheckResult> subResultPointer = this->check(env, pathFormula.getSubformula());
            ExplicitQualitativeCheckResult const& subResult = subResultPointer->asExplicitQualitativeCheckResult();

            auto ret = storm::modelchecker::helper::SparseSmgRpatlHelper<ValueType>::computeBoundedGloballyProbabilities(env, storm::solver::SolveGoal<ValueType>(this->getModel(), checkTask), this->getModel().getTransitionMatrix(), this->getModel().getBackwardTransitions(), subResult.getTruthValuesVector(), checkTask.isQualitativeSet(), statesOfCoalition, checkTask.isProduceSchedulersSet(), checkTask.getHint(), pathFormula.getNonStrictLowerBound<uint64_t>(), pathFormula.getNonStrictUpperBound<uint64_t>());
            std::unique_ptr<CheckResult> result(new ExplicitQuantitativeCheckResult<ValueType>(std::move(ret.values)));
            if(checkTask.isShieldingTask()) {
                storm::storage::BitVector allStatesBv = storm::storage::BitVector(this->getModel().getTransitionMatrix().getRowGroupCount(), true);
                tempest::shields::createShield<ValueType>(std::make_shared<storm::models::sparse::Smg<ValueType>>(this->getModel()), std::move(ret.choiceValues), checkTask.getShieldingExpression(), checkTask.getOptimizationDirection(), allStatesBv, ~statesOfCoalition);
            }
            return result;
        }

        template<typename ModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<ModelType>::computeBoundedUntilProbabilities(Environment const& env, CheckTask<storm::logic::BoundedUntilFormula, ValueType> const& checkTask) {
            storm::logic::BoundedUntilFormula const& pathFormula = checkTask.getFormula();
            STORM_LOG_THROW(checkTask.isOptimizationDirectionSet(), storm::exceptions::InvalidPropertyException, "Formula needs to specify whether minimal or maximal values are to be computed on nondeterministic model.");
            // checks for bounds
            STORM_LOG_THROW(pathFormula.hasUpperBound(), storm::exceptions::InvalidPropertyException, "Formula needs to have (a single) upper step bound.");
            STORM_LOG_THROW(pathFormula.hasIntegerLowerBound(), storm::exceptions::InvalidPropertyException, "Formula lower step bound must be discrete.");
            STORM_LOG_THROW(pathFormula.hasIntegerUpperBound(), storm::exceptions::InvalidPropertyException, "Formula upper step bound must be discrete.");

            std::unique_ptr<CheckResult> leftResultPointer = this->check(env, pathFormula.getLeftSubformula());
            std::unique_ptr<CheckResult> rightResultPointer = this->check(env, pathFormula.getRightSubformula());
            ExplicitQualitativeCheckResult const& leftResult = leftResultPointer->asExplicitQualitativeCheckResult();
            ExplicitQualitativeCheckResult const& rightResult = rightResultPointer->asExplicitQualitativeCheckResult();

            auto ret = storm::modelchecker::helper::SparseSmgRpatlHelper<ValueType>::computeBoundedUntilProbabilities(env, storm::solver::SolveGoal<ValueType>(this->getModel(), checkTask), this->getModel().getTransitionMatrix(), this->getModel().getBackwardTransitions(), leftResult.getTruthValuesVector(), rightResult.getTruthValuesVector(), checkTask.isQualitativeSet(), statesOfCoalition, checkTask.isProduceSchedulersSet(), checkTask.getHint(), pathFormula.getNonStrictLowerBound<uint64_t>(), pathFormula.getNonStrictUpperBound<uint64_t>());
            std::unique_ptr<CheckResult> result(new ExplicitQuantitativeCheckResult<ValueType>(std::move(ret.values)));
            if(checkTask.isShieldingTask()) {
                tempest::shields::createShield<ValueType>(std::make_shared<storm::models::sparse::Smg<ValueType>>(this->getModel()), std::move(ret.choiceValues), checkTask.getShieldingExpression(), checkTask.getOptimizationDirection(), std::move(ret.relevantStates), ~statesOfCoalition);
            }
            return result;
        }

        template<typename SparseSmgModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<SparseSmgModelType>::computeLongRunAverageProbabilities(Environment const& env, CheckTask<storm::logic::StateFormula, ValueType> const& checkTask) {
            STORM_LOG_THROW(false, storm::exceptions::NotImplementedException, "NYI");
        }

        template<typename SparseSmgModelType>
        std::unique_ptr<CheckResult> SparseSmgRpatlModelChecker<SparseSmgModelType>::computeLongRunAverageRewards(Environment const& env, storm::logic::RewardMeasureType rewardMeasureType, CheckTask<storm::logic::LongRunAverageRewardFormula, ValueType> const& checkTask) {
            auto rewardModel = storm::utility::createFilteredRewardModel(this->getModel(), checkTask);
            storm::modelchecker::helper::SparseNondeterministicGameInfiniteHorizonHelper<ValueType> helper(this->getModel().getTransitionMatrix(), statesOfCoalition);
            storm::modelchecker::helper::setInformationFromCheckTaskNondeterministic(helper, checkTask, this->getModel());
            auto values = helper.computeLongRunAverageRewards(env, rewardModel.get());

            std::unique_ptr<CheckResult> result(new ExplicitQuantitativeCheckResult<ValueType>(std::move(values)));
            if(checkTask.isShieldingTask()) {
                storm::storage::BitVector allStatesBv = storm::storage::BitVector(this->getModel().getTransitionMatrix().getRowGroupCount(), true);
                tempest::shields::createQuantitativeShield<ValueType>(std::make_shared<storm::models::sparse::Smg<ValueType>>(this->getModel()), helper.getChoiceValues(), checkTask.getShieldingExpression(), checkTask.getOptimizationDirection(), allStatesBv, statesOfCoalition);
            } else if (checkTask.isProduceSchedulersSet()) {
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
