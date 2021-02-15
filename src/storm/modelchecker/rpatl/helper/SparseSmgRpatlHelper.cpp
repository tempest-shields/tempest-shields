#include "SparseSmgRpatlHelper.h"

#include "storm/environment/Environment.h"
#include "storm/environment/solver/MultiplierEnvironment.h"
#include "storm/environment/solver/MinMaxSolverEnvironment.h"
#include "storm/solver/MinMaxLinearEquationSolver.h"
#include "storm/utility/vector.h"

#include "storm/modelchecker/rpatl/helper/internal/GameViHelper.h"


namespace storm {
    namespace modelchecker {
        namespace helper {

            template<typename ValueType>
            MDPSparseModelCheckingHelperReturnType<ValueType> SparseSmgRpatlHelper<ValueType>::computeUntilProbabilities(Environment const& env, storm::solver::SolveGoal<ValueType>&& goal, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& phiStates, storm::storage::BitVector const& psiStates, bool qualitative, storm::storage::BitVector statesOfCoalition, bool produceScheduler, ModelCheckerHint const& hint) {
                // TODO add Kwiatkowska original reference
                // TODO FIX solver min max mess

                auto solverEnv = env;
                solverEnv.solver().minMax().setMethod(storm::solver::MinMaxMethod::ValueIteration, false);

                // Initialize the solution vector.
                std::vector<ValueType> x = std::vector<ValueType>(transitionMatrix.getRowGroupCount() - psiStates.getNumberOfSetBits(), storm::utility::zero<ValueType>());

                // Relevant states are those states which are phiStates and not PsiStates.
                storm::storage::BitVector relevantStates = phiStates & ~psiStates;

                std::vector<ValueType> b = transitionMatrix.getConstrainedRowGroupSumVector(relevantStates, psiStates);

                // Reduce the matrix to relevant states
                storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, relevantStates, relevantStates, false);

                storm::storage::BitVector clippedStatesOfCoalition(relevantStates.getNumberOfSetBits());
                clippedStatesOfCoalition.setClippedStatesOfCoalition(relevantStates, statesOfCoalition);
                clippedStatesOfCoalition.complement();

                storm::modelchecker::helper::internal::GameViHelper<ValueType> viHelper(submatrix, clippedStatesOfCoalition);
                std::unique_ptr<storm::storage::Scheduler<ValueType>> scheduler;
                if (produceScheduler) {
                    viHelper.setProduceScheduler(true);
                }

                viHelper.performValueIteration(env, x, b, goal.direction());
                viHelper.fillResultVector(x, relevantStates, psiStates);

                if (produceScheduler) {
                    scheduler = std::make_unique<storm::storage::Scheduler<ValueType>>(expandScheduler(viHelper.extractScheduler(), psiStates));
                }
                return MDPSparseModelCheckingHelperReturnType<ValueType>(std::move(x), std::move(scheduler));
            }

            template<typename ValueType>
            storm::storage::Scheduler<ValueType> SparseSmgRpatlHelper<ValueType>::expandScheduler(storm::storage::Scheduler<ValueType> scheduler, storm::storage::BitVector psiStates) {
                for(uint i = 0; i < 2; i++) {
                }
                storm::storage::Scheduler<ValueType> completeScheduler(psiStates.size());
                uint_fast64_t maybeStatesCounter = 0;
                for(uint stateCounter = 0; stateCounter < psiStates.size(); stateCounter++) {
                    if(psiStates.get(stateCounter)) {
                        completeScheduler.setChoice(0, stateCounter);
                    } else {
                        completeScheduler.setChoice(scheduler.getChoice(maybeStatesCounter), stateCounter);
                        maybeStatesCounter++;
                    }
                }
                return completeScheduler;
            }

            template class SparseSmgRpatlHelper<double>;
#ifdef STORM_HAVE_CARL
            template class SparseSmgRpatlHelper<storm::RationalNumber>;
#endif
        }
    }
}
