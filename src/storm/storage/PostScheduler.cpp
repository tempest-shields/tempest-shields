#include "storm/utility/vector.h"
#include "storm/storage/PostScheduler.h"

#include "storm/utility/macros.h"
#include "storm/exceptions/NotImplementedException.h"
#include <boost/algorithm/string/join.hpp>

namespace storm {
    namespace storage {

        template <typename ValueType>
        PostScheduler<ValueType>::PostScheduler(uint_fast64_t numberOfModelStates, std::vector<uint_fast64_t> numberOfChoicesPerState, boost::optional<storm::storage::MemoryStructure> const& memoryStructure) : Scheduler<ValueType>(numberOfModelStates, memoryStructure) {
            STORM_LOG_DEBUG(numberOfChoicesPerState.size() << " " << numberOfModelStates);
            STORM_LOG_ASSERT(numberOfChoicesPerState.size() == numberOfModelStates, "Need to know amount of choices per model state");
            uint_fast64_t numOfMemoryStates = memoryStructure ? memoryStructure->getNumberOfStates() : 1;
            schedulerChoiceMapping = std::vector<std::vector<std::vector<SchedulerChoice<ValueType>>>>(numOfMemoryStates, std::vector<std::vector<SchedulerChoice<ValueType>>>(numberOfModelStates));
            for(uint state = 0; state < numberOfModelStates; state++) {
                schedulerChoiceMapping[0][state].resize(numberOfChoicesPerState[state]);
            }
            numberOfChoices = 0;
            for(std::vector<uint_fast64_t>::iterator it = numberOfChoicesPerState.begin(); it != numberOfChoicesPerState.end(); ++it)
                numberOfChoices += *it;
            this->numOfUndefinedChoices = numOfMemoryStates * numberOfChoices;
            this->numOfDeterministicChoices = 0;
        }

        template <typename ValueType>
        PostScheduler<ValueType>::PostScheduler(uint_fast64_t numberOfModelStates, std::vector<uint_fast64_t> numberOfChoicesPerState, boost::optional<storm::storage::MemoryStructure>&& memoryStructure) : Scheduler<ValueType>(numberOfModelStates, std::move(memoryStructure)) {
            STORM_LOG_ASSERT(numberOfChoicesPerState.size() == numberOfModelStates, "Need to know amount of choices per model state");
            uint_fast64_t numOfMemoryStates = memoryStructure ? memoryStructure->getNumberOfStates() : 1;
            schedulerChoiceMapping = std::vector<std::vector<std::vector<SchedulerChoice<ValueType>>>>(numOfMemoryStates, std::vector<std::vector<SchedulerChoice<ValueType>>>(numberOfModelStates));
            for(uint state = 0; state < numberOfModelStates; state++) {
                schedulerChoiceMapping[0][state].resize(numberOfChoicesPerState[state]);
            }
            numberOfChoices = 0;
            for(std::vector<uint_fast64_t>::iterator it = numberOfChoicesPerState.begin(); it != numberOfChoicesPerState.end(); ++it)
                numberOfChoices += *it;
            this->numOfUndefinedChoices = numOfMemoryStates * numberOfChoices;
            this->numOfDeterministicChoices = 0;
        }

        template <typename ValueType>
        void PostScheduler<ValueType>::setChoice(OldChoice const& oldChoice, SchedulerChoice<ValueType> const& newChoice, uint_fast64_t modelState, uint_fast64_t memoryState) {
            STORM_LOG_ASSERT(memoryState == 0, "Currently we do not support PostScheduler with memory");
            STORM_LOG_ASSERT(modelState < schedulerChoiceMapping[memoryState].size(), "Illegal model state index");

            //auto& schedulerChoice = schedulerChoiceMapping[memoryState][modelState];
            //if (schedulerChoice.isDefined()) {
            //    if (!choice.isDefined()) {
            //        ++numOfUndefinedChoices;
            //    }
            //} else {
            //    if (choice.isDefined()) {
            //       assert(numOfUndefinedChoices > 0);
            //        --numOfUndefinedChoices;
            //    }
            //}
            //if (schedulerChoice.isDeterministic()) {
            //    if (!choice.isDeterministic()) {
            //        assert(numOfDeterministicChoices > 0);
            //        --numOfDeterministicChoices;
            //    }
            //} else {
            //    if (choice.isDeterministic()) {
            //        ++numOfDeterministicChoices;
            //    }
            //}
            schedulerChoiceMapping[memoryState][modelState][oldChoice] = newChoice;
        }

        template <typename ValueType>
        SchedulerChoice<ValueType> const& PostScheduler<ValueType>::getChoice(uint_fast64_t modelState, OldChoice oldChoice, uint_fast64_t memoryState)  {
            STORM_LOG_ASSERT(memoryState < this->getNumberOfMemoryStates(), "Illegal memory state index");
            STORM_LOG_ASSERT(modelState < schedulerChoiceMapping[memoryState].size(), "Illegal model state index");
            return schedulerChoiceMapping[memoryState][modelState][oldChoice];
        }


        template <typename ValueType>
        bool PostScheduler<ValueType>::isDeterministicScheduler() const {
            return true;
        }

        template <typename ValueType>
        bool PostScheduler<ValueType>::isMemorylessScheduler() const {
            return true;
        }

        template <typename ValueType>
        void PostScheduler<ValueType>::printToStream(std::ostream& out, std::shared_ptr<storm::logic::ShieldExpression const> shieldingExpression, std::shared_ptr<storm::models::sparse::Model<ValueType>> model, bool skipUniqueChoices) const {
            STORM_LOG_THROW(this->isMemorylessScheduler(), storm::exceptions::InvalidOperationException, "The given scheduler is incompatible.");

            bool const stateValuationsGiven = model != nullptr && model->hasStateValuations();
            bool const choiceLabelsGiven = model != nullptr && model->hasChoiceLabeling();
            bool const choiceOriginsGiven = model != nullptr && model->hasChoiceOrigins();
            uint_fast64_t widthOfStates = std::to_string(schedulerChoiceMapping.front().size()).length();
            if (stateValuationsGiven) {
                widthOfStates += model->getStateValuations().getStateInfo(schedulerChoiceMapping.front().size() - 1).length() + 5;
            }
            widthOfStates = std::max(widthOfStates, (uint_fast64_t)12);
            out << "___________________________________________________________________" << std::endl;
            out << shieldingExpression->prettify() << std::endl;
            STORM_LOG_WARN_COND(!(skipUniqueChoices && model == nullptr), "Can not skip unique choices if the model is not given.");
            out << std::setw(widthOfStates) << "model state:" << "    " << "correction";
            if(choiceLabelsGiven) {
                out << " [<action> {action label}: <corrected action> {corrected action label}]";
            } else {
                out << " [<action>: (<corrected action>)}";
            }
            out << ":" << std::endl;
            uint_fast64_t numOfSkippedStatesWithUniqueChoice = 0;
            for (uint_fast64_t state = 0; state < schedulerChoiceMapping.front().size(); ++state) {
                std::stringstream stateString;
                // Print the state info
                if (stateValuationsGiven) {
                    stateString << std::setw(widthOfStates)  << (std::to_string(state) + ": " + model->getStateValuations().getStateInfo(state));
                } else {
                    stateString << std::setw(widthOfStates) << state;
                }
                stateString << "    ";

                bool firstChoiceIndex = true;
                for(uint choiceIndex = 0; choiceIndex < schedulerChoiceMapping[0][state].size(); choiceIndex++) {
                    SchedulerChoice<ValueType> const& choice = schedulerChoiceMapping[0][state][choiceIndex];
                    if(firstChoiceIndex) {
                        firstChoiceIndex = false;
                    } else {
                        stateString << ";    ";
                    }
                    if (choice.isDefined()) {
                        auto choiceProbPair = *(choice.getChoiceAsDistribution().begin());
                        if(choiceLabelsGiven) {
                            auto choiceLabels = model->getChoiceLabeling().getLabelsOfChoice(model->getTransitionMatrix().getRowGroupIndices()[state] + choiceIndex);
                            stateString << std::to_string(choiceIndex) << " {" << boost::join(choiceLabels, ", ") << "}: ";
                        } else {
                            stateString << std::to_string(choiceIndex) << ": ";
                        }
                        //stateString << choiceProbPair.second << ": (";
                        if (choiceOriginsGiven) {
                            stateString << model->getChoiceOrigins()->getChoiceInfo(model->getTransitionMatrix().getRowGroupIndices()[state] + choiceProbPair.first);
                        } else {
                            stateString << choiceProbPair.first;
                        }
                        if (choiceLabelsGiven) {
                            auto choiceLabels = model->getChoiceLabeling().getLabelsOfChoice(model->getTransitionMatrix().getRowGroupIndices()[state] + choiceProbPair.first);
                            stateString << " {" << boost::join(choiceLabels, ", ") << "}";
                        }

                    } else {
                        if(!this->printUndefinedChoices) goto skipStatesWithUndefinedChoices;
                        stateString << "undefined.";
                    }
                    // Todo: print memory updates
                }
                out << stateString.str() << std::endl;
                // jump to label if we find one undefined choice.
                skipStatesWithUndefinedChoices:;
            }
        }

        template class PostScheduler<double>;
#ifdef STORM_HAVE_CARL
        template class PostScheduler<storm::RationalNumber>;
#endif
    }
}
