#include "storm/utility/vector.h"
#include "storm/storage/PostScheduler.h"

#include "storm/utility/macros.h"
#include "storm/exceptions/NotImplementedException.h"
#include <boost/algorithm/string/join.hpp>

namespace storm {
    namespace storage {

        template <typename ValueType>
        PostScheduler<ValueType>::PostScheduler(uint_fast64_t numberOfModelStates, std::vector<uint_fast64_t> numberOfChoicesPerState, boost::optional<storm::storage::MemoryStructure> const& memoryStructure) : memoryStructure(memoryStructure) {
            //STORM_LOG_ASSERT(numberOfChoicesPerState.size() == numberOfModelStates, "Need to know amount of choices per model state");
            uint_fast64_t numOfMemoryStates = memoryStructure ? memoryStructure->getNumberOfStates() : 1;
            schedulerChoiceMapping = std::vector<std::vector<PostSchedulerChoice<ValueType>>>(numOfMemoryStates, std::vector<PostSchedulerChoice<ValueType>>(numberOfModelStates));
            numberOfChoices = 0;
            for(std::vector<uint_fast64_t>::iterator it = numberOfChoicesPerState.begin(); it != numberOfChoicesPerState.end(); ++it)
                numberOfChoices += *it;
            this->numOfUndefinedChoices = numOfMemoryStates * numberOfChoices;
            this->numOfDeterministicChoices = 0;
        }

        template <typename ValueType>
        PostScheduler<ValueType>::PostScheduler(uint_fast64_t numberOfModelStates, std::vector<uint_fast64_t> numberOfChoicesPerState, boost::optional<storm::storage::MemoryStructure>&& memoryStructure) : memoryStructure(std::move(memoryStructure)) {
            STORM_LOG_ASSERT(numberOfChoicesPerState.size() == numberOfModelStates, "Need to know amount of choices per model state");
            uint_fast64_t numOfMemoryStates = memoryStructure ? memoryStructure->getNumberOfStates() : 1;
            schedulerChoiceMapping = std::vector<std::vector<PostSchedulerChoice<ValueType>>>(numOfMemoryStates, std::vector<PostSchedulerChoice<ValueType>>(numberOfModelStates));
            numberOfChoices = 0;
            for(std::vector<uint_fast64_t>::iterator it = numberOfChoicesPerState.begin(); it != numberOfChoicesPerState.end(); ++it)
                numberOfChoices += *it;
            this->numOfUndefinedChoices = numOfMemoryStates * numberOfChoices;
            this->numOfDeterministicChoices = 0;
        }

        template <typename ValueType>
        void PostScheduler<ValueType>::setChoice(PostSchedulerChoice<ValueType> const& choice, uint_fast64_t modelState, uint_fast64_t memoryState) {
            STORM_LOG_ASSERT(memoryState == 0, "Currently we do not support PostScheduler with memory");
            STORM_LOG_ASSERT(modelState < schedulerChoiceMapping[memoryState].size(), "Illegal model state index");

            schedulerChoiceMapping[memoryState][modelState] = choice;
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
                PostSchedulerChoice<ValueType> const& choices = schedulerChoiceMapping[0][state];
                if(choices.isEmpty() && !printUndefinedChoices) continue;

                std::stringstream stateString;

                // Print the state info
                if (stateValuationsGiven) {
                    stateString << std::setw(widthOfStates)  << (std::to_string(state) + ": " + model->getStateValuations().getStateInfo(state));
                } else {
                    stateString << std::setw(widthOfStates) << state;
                }
                stateString << "    ";


                bool firstChoiceIndex = true;
                for(auto const& choiceMap : choices.getChoiceMap()) {
                    if(firstChoiceIndex) {
                        firstChoiceIndex = false;
                    } else {
                        stateString << ";    ";
                    }

                    if(choiceLabelsGiven) {
                        auto choiceLabels = model->getChoiceLabeling().getLabelsOfChoice(model->getTransitionMatrix().getRowGroupIndices()[state] + std::get<0>(choiceMap));
                        stateString << std::to_string(std::get<0>(choiceMap)) << " {" << boost::join(choiceLabels, ", ") << "}: ";
                    } else {
                        stateString << std::to_string(std::get<0>(choiceMap)) << ": ";
                    }

                    if (choiceOriginsGiven) {
                        stateString << model->getChoiceOrigins()->getChoiceInfo(model->getTransitionMatrix().getRowGroupIndices()[state] + std::get<1>(choiceMap));
                    } else {
                        stateString << std::to_string(std::get<1>(choiceMap));
                    }
                    if (choiceLabelsGiven) {
                        auto choiceLabels = model->getChoiceLabeling().getLabelsOfChoice(model->getTransitionMatrix().getRowGroupIndices()[state] + std::get<1>(choiceMap));
                        stateString << " {" << boost::join(choiceLabels, ", ") << "}";
                    }

                    // Todo: print memory updates
                }
                out << stateString.str() << std::endl;
                // jump to label if we find one undefined choice.
                //skipStatesWithUndefinedChoices:;
            }
            out << "___________________________________________________________________" << std::endl;
        }

        template class PostScheduler<double>;
#ifdef STORM_HAVE_CARL
        template class PostScheduler<storm::RationalNumber>;
#endif
    }
}
