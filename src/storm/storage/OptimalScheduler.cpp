#include "storm/utility/vector.h"
#include "storm/storage/OptimalScheduler.h"

#include "storm/utility/macros.h"
#include "storm/exceptions/NotImplementedException.h"
#include <boost/algorithm/string/join.hpp>

namespace storm {
    namespace storage {
        template <typename ValueType>
        OptimalScheduler<ValueType>::OptimalScheduler(uint_fast64_t numberOfModelStates, boost::optional<storm::storage::MemoryStructure> const& memoryStructure) : Scheduler<ValueType>(numberOfModelStates, memoryStructure) {
        }

        template <typename ValueType>
        OptimalScheduler<ValueType>::OptimalScheduler(uint_fast64_t numberOfModelStates, boost::optional<storm::storage::MemoryStructure>&& memoryStructure) : Scheduler<ValueType>(numberOfModelStates, std::move(memoryStructure)) {
        }

        template <typename ValueType>
        void OptimalScheduler<ValueType>::printToStream(std::ostream& out, std::shared_ptr<storm::logic::ShieldExpression const> shieldingExpression, std::shared_ptr<storm::models::sparse::Model<ValueType>> model, bool skipUniqueChoices) const {
            STORM_LOG_THROW(model == nullptr || model->getNumberOfStates() == this->schedulerChoices.front().size(), storm::exceptions::InvalidOperationException, "The given model is not compatible with this scheduler.");
            STORM_LOG_THROW(this->isMemorylessScheduler(), storm::exceptions::InvalidOperationException, "The given scheduler is incompatible.");

            bool const stateValuationsGiven = model != nullptr && model->hasStateValuations();
            bool const choiceLabelsGiven = model != nullptr && model->hasChoiceLabeling();
            bool const choiceOriginsGiven = model != nullptr && model->hasChoiceOrigins();
            uint_fast64_t widthOfStates = std::to_string(this->schedulerChoices.front().size()).length();
            if (stateValuationsGiven) {
                widthOfStates += model->getStateValuations().getStateInfo(this->schedulerChoices.front().size() - 1).length() + 5;
            }
            widthOfStates = std::max(widthOfStates, (uint_fast64_t)12);
            uint_fast64_t numOfSkippedStatesWithUniqueChoice = 0;

            out << "___________________________________________________________________" << std::endl;
            out << shieldingExpression->prettify() << std::endl;
            STORM_LOG_WARN_COND(!(skipUniqueChoices && model == nullptr), "Can not skip unique choices if the model is not given.");
            out << std::setw(widthOfStates) << "model state:" << "    " << "choice";
            if(choiceLabelsGiven) {
                out << " [<value>: (<action {action label})>]";
            } else {
                out << " [<value>: (<action>)}";
            }
            out << ":" << std::endl;
            for (uint_fast64_t state = 0; state < this->schedulerChoices.front().size(); ++state) {
                std::stringstream stateString;
                // Check whether the state is skipped
                if (skipUniqueChoices && model != nullptr && model->getTransitionMatrix().getRowGroupSize(state) == 1) {
                    ++numOfSkippedStatesWithUniqueChoice;
                    continue;
                }

                // Print the state info
                if (stateValuationsGiven) {
                    stateString << std::setw(widthOfStates)  << (std::to_string(state) + ": " + model->getStateValuations().getStateInfo(state));
                } else {
                    stateString << std::setw(widthOfStates) << state;
                }
                stateString << "    ";

                bool firstMemoryState = true;
                for (uint_fast64_t memoryState = 0; memoryState < this->getNumberOfMemoryStates(); ++memoryState) {
                    // Indent if this is not the first memory state
                    if (firstMemoryState) {
                        firstMemoryState = false;
                    } else {
                        stateString << std::setw(widthOfStates) << "";
                        stateString << "    ";
                    }

                    // Print choice info
                    SchedulerChoice<ValueType> const& choice = this->schedulerChoices[memoryState][state];
                    if (choice.isDefined()) {
                        bool firstChoice = true;
                        for (auto const& choiceProbPair : choice.getChoiceAsDistribution()) {
                            if (firstChoice) {
                                firstChoice = false;
                            } else {
                                stateString << ";    ";
                            }
                            stateString << choiceProbPair.second << ": (";
                            if (choiceOriginsGiven) {
                                stateString << model->getChoiceOrigins()->getChoiceInfo(model->getTransitionMatrix().getRowGroupIndices()[state] + choiceProbPair.first);
                            } else {
                                stateString << choiceProbPair.first;
                            }
                            if (choiceLabelsGiven) {
                                auto choiceLabels = model->getChoiceLabeling().getLabelsOfChoice(model->getTransitionMatrix().getRowGroupIndices()[state] + choiceProbPair.first);
                                stateString << " {" << boost::join(choiceLabels, ", ") << "}";
                            }
                            stateString << ")";
                        }
                    } else {
                        if(!this->printUndefinedChoices) continue;
                        stateString << "undefined.";
                    }

                    // Todo: print memory updates
                    out << stateString.str();
                    out << std::endl;
                }
            }
            if (numOfSkippedStatesWithUniqueChoice > 0) {
                out << "Skipped " << numOfSkippedStatesWithUniqueChoice << " deterministic states with unique choice." << std::endl;
            }
            out << "___________________________________________________________________" << std::endl;
        }

        template class OptimalScheduler<double>;
#ifdef STORM_HAVE_CARL
        template class OptimalScheduler<storm::RationalNumber>;
#endif
    }
}
