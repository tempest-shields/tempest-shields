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
                std::vector<ValueType> b = transitionMatrix.getConstrainedRowGroupSumVector(~psiStates, psiStates);

                // Reduce matrix to ~Prob1 states-
                //STORM_LOG_DEBUG("\n" << transitionMatrix);
                storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, ~psiStates, ~psiStates, false);
                //STORM_LOG_DEBUG("\n" << submatrix);


                //STORM_LOG_DEBUG("x = " << storm::utility::vector::toString(x));
                //STORM_LOG_DEBUG("b = " << storm::utility::vector::toString(b));

                storm::storage::BitVector clippedStatesOfCoalition(statesOfCoalition.size() - psiStates.getNumberOfSetBits());
                //STORM_LOG_DEBUG(psiStates);
                //STORM_LOG_DEBUG(statesOfCoalition);
                //STORM_LOG_DEBUG(clippedStatesOfCoalition);

                // TODO move this to BitVector-class
                auto clippedStatesCounter = 0;
                for(uint i = 0; i < psiStates.size(); i++) {
                    std::cout << i << " : " << psiStates.get(i) << "  -> " << statesOfCoalition[i] << std::endl;
                    if(!psiStates.get(i)) {
                        clippedStatesOfCoalition.set(clippedStatesCounter, statesOfCoalition[i]);
                        clippedStatesCounter++;
                    }
                }
                //STORM_LOG_DEBUG(clippedStatesOfCoalition);
                clippedStatesOfCoalition.complement();
                //STORM_LOG_DEBUG(clippedStatesOfCoalition);

                storm::modelchecker::helper::internal::GameViHelper<ValueType> viHelper(submatrix, clippedStatesOfCoalition);
                std::unique_ptr<storm::storage::Scheduler<ValueType>> scheduler;
                if (produceScheduler) {
                    viHelper.setProduceScheduler(true);
                }

                viHelper.performValueIteration(env, x, b, goal.direction());

                STORM_LOG_DEBUG(storm::utility::vector::toString(x));
                if (produceScheduler) {
                    scheduler = std::make_unique<storm::storage::Scheduler<ValueType>>(expandScheduler(viHelper.extractScheduler(), psiStates));
                    STORM_LOG_DEBUG("IsPartial?" << scheduler->isPartialScheduler());
                }
                return MDPSparseModelCheckingHelperReturnType<ValueType>(std::move(x), std::move(scheduler));
            }

            template<typename ValueType>
            storm::storage::Scheduler<ValueType> SparseSmgRpatlHelper<ValueType>::expandScheduler(storm::storage::Scheduler<ValueType> scheduler, storm::storage::BitVector psiStates) {
                //STORM_LOG_DEBUG(psiStates.size());
                for(uint i = 0; i < 2; i++) {
                    //STORM_LOG_DEBUG(scheduler.getChoice(i));
                }
                storm::storage::Scheduler<ValueType> completeScheduler(psiStates.size());
                uint_fast64_t maybeStatesCounter = 0;
                for(uint stateCounter = 0; stateCounter < psiStates.size(); stateCounter++) {
                    //STORM_LOG_DEBUG(stateCounter << " : " << psiStates.get(stateCounter));
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
