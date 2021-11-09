#include "storm/utility/vector.h"
#include "storm/storage/PreScheduler.h"

#include "storm/utility/macros.h"
#include "storm/exceptions/NotImplementedException.h"
#include <boost/algorithm/string/join.hpp>

namespace storm {
    namespace storage {
        template <typename ValueType>
        PreScheduler<ValueType>::PreScheduler(uint_fast64_t numberOfModelStates, boost::optional<storm::storage::MemoryStructure> const& memoryStructure) : memoryStructure(memoryStructure) {
            uint_fast64_t numOfMemoryStates = memoryStructure ? memoryStructure->getNumberOfStates() : 1;
            schedulerChoices = std::vector<std::vector<PreSchedulerChoice<ValueType>>>(numOfMemoryStates, std::vector<PreSchedulerChoice<ValueType>>(numberOfModelStates));
            //dontCareStates =  std::vector<storm::storage::BitVector>(numOfMemoryStates, storm::storage::BitVector(numberOfModelStates, false));
            numOfUndefinedChoices = numOfMemoryStates * numberOfModelStates;
            numOfDeterministicChoices = 0;
            numOfDontCareStates = 0;
        }

        template <typename ValueType>
        PreScheduler<ValueType>::PreScheduler(uint_fast64_t numberOfModelStates, boost::optional<storm::storage::MemoryStructure>&& memoryStructure) : memoryStructure(std::move(memoryStructure)) {
        }

        template <typename ValueType>
        bool PreScheduler<ValueType>::isMemorylessScheduler() const {
            return getNumberOfMemoryStates() == 1;
        }

        template <typename ValueType>
        uint_fast64_t PreScheduler<ValueType>::getNumberOfMemoryStates() const {
            return memoryStructure ? memoryStructure->getNumberOfStates() : 1;
        }

        template <typename ValueType>
        void PreScheduler<ValueType>::setChoice(PreSchedulerChoice<ValueType> const& choice, uint_fast64_t modelState, uint_fast64_t memoryState) {
            STORM_LOG_ASSERT(memoryState < this->getNumberOfMemoryStates(), "Illegal memory state index");
            STORM_LOG_ASSERT(modelState < this->schedulerChoices[memoryState].size(), "Illegal model state index");

            auto& schedulerChoice = schedulerChoices[memoryState][modelState];
            schedulerChoice = choice;
        }

        template <typename ValueType>
        void PreScheduler<ValueType>::printToStream(std::ostream& out, std::shared_ptr<storm::logic::ShieldExpression const> shieldingExpression, std::shared_ptr<storm::models::sparse::Model<ValueType>> model, bool skipUniqueChoices) const {
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
            out << std::setw(widthOfStates) << "model state:" << "    " << "choice(s)";
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
                    PreSchedulerChoice<ValueType> const& choices = this->schedulerChoices[memoryState][state];
                    if (!choices.isEmpty()) {
                        bool firstChoice = true;
                        for (auto const& choiceProbPair : choices.getChoiceMap()) {
                            if (firstChoice) {
                                firstChoice = false;
                            } else {
                                stateString << ";    ";
                            }
                            stateString << std::get<0>(choiceProbPair) << ": (";
                            if (choiceOriginsGiven) {
                                stateString << model->getChoiceOrigins()->getChoiceInfo(model->getTransitionMatrix().getRowGroupIndices()[state] + std::get<1>(choiceProbPair));
                            } else {
                                stateString << std::get<1>(choiceProbPair);
                            }
                            if (choiceLabelsGiven) {
                                auto choiceLabels = model->getChoiceLabeling().getLabelsOfChoice(model->getTransitionMatrix().getRowGroupIndices()[state] + std::get<1>(choiceProbPair));
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

        template class PreScheduler<double>;
#ifdef STORM_HAVE_CARL
        template class PreScheduler<storm::RationalNumber>;
#endif
    }
}
