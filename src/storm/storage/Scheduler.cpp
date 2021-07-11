#include "storm/utility/vector.h"
#include "storm/storage/Scheduler.h"

#include "storm/utility/macros.h"
#include "storm/adapters/JsonAdapter.h"
#include "storm/exceptions/NotImplementedException.h"
#include <boost/algorithm/string/join.hpp>

namespace storm {
    namespace storage {

        template <typename ValueType>
        Scheduler<ValueType>::Scheduler(uint_fast64_t numberOfModelStates, boost::optional<storm::storage::MemoryStructure> const& memoryStructure) : memoryStructure(memoryStructure) {
            uint_fast64_t numOfMemoryStates = memoryStructure ? memoryStructure->getNumberOfStates() : 1;
            schedulerChoices = std::vector<std::vector<SchedulerChoice<ValueType>>>(numOfMemoryStates, std::vector<SchedulerChoice<ValueType>>(numberOfModelStates));
            numOfUndefinedChoices = numOfMemoryStates * numberOfModelStates;
            numOfDeterministicChoices = 0;
        }

        template <typename ValueType>
        Scheduler<ValueType>::Scheduler(uint_fast64_t numberOfModelStates, boost::optional<storm::storage::MemoryStructure>&& memoryStructure) : memoryStructure(std::move(memoryStructure)) {
            uint_fast64_t numOfMemoryStates = this->memoryStructure ? this->memoryStructure->getNumberOfStates() : 1;
            schedulerChoices = std::vector<std::vector<SchedulerChoice<ValueType>>>(numOfMemoryStates, std::vector<SchedulerChoice<ValueType>>(numberOfModelStates));
            numOfUndefinedChoices = numOfMemoryStates * numberOfModelStates;
            numOfDeterministicChoices = 0;
        }

        template <typename ValueType>
        void Scheduler<ValueType>::setChoice(SchedulerChoice<ValueType> const& choice, uint_fast64_t modelState, uint_fast64_t memoryState) {
            STORM_LOG_ASSERT(memoryState < getNumberOfMemoryStates(), "Illegal memory state index");
            STORM_LOG_ASSERT(modelState < schedulerChoices[memoryState].size(), "Illegal model state index");
            auto& schedulerChoice = schedulerChoices[memoryState][modelState];
            if (schedulerChoice.isDefined()) {
                if (!choice.isDefined()) {
                    ++numOfUndefinedChoices;
                }
            } else {
                if (choice.isDefined()) {
                   assert(numOfUndefinedChoices > 0);
                    --numOfUndefinedChoices;
                }
            }
            if (schedulerChoice.isDeterministic()) {
                if (!choice.isDeterministic()) {
                    assert(numOfDeterministicChoices > 0);
                    --numOfDeterministicChoices;
                }
            } else {
                if (choice.isDeterministic()) {
                    ++numOfDeterministicChoices;
                }
            }

            schedulerChoice = choice;
        }

        template <typename ValueType>
        bool Scheduler<ValueType>::isChoiceSelected(BitVector const& selectedStates, uint64_t memoryState) const {
            for (auto selectedState : selectedStates) {
                auto& schedulerChoice = schedulerChoices[memoryState][selectedState];
                if (!schedulerChoice.isDefined()) {
                    return false;
                }
            }
            return true;
        }

        template <typename ValueType>
        void Scheduler<ValueType>::clearChoice(uint_fast64_t modelState, uint_fast64_t memoryState) {
            STORM_LOG_ASSERT(memoryState < getNumberOfMemoryStates(), "Illegal memory state index");
            STORM_LOG_ASSERT(modelState < schedulerChoices[memoryState].size(), "Illegal model state index");
            setChoice(SchedulerChoice<ValueType>(), modelState, memoryState);
        }

        template <typename ValueType>
        SchedulerChoice<ValueType> const& Scheduler<ValueType>::getChoice(uint_fast64_t modelState, uint_fast64_t memoryState) const {
            STORM_LOG_ASSERT(memoryState < getNumberOfMemoryStates(), "Illegal memory state index");
            STORM_LOG_ASSERT(modelState < schedulerChoices[memoryState].size(), "Illegal model state index");
            return schedulerChoices[memoryState][modelState];
        }

        template<typename ValueType>
        storm::storage::BitVector Scheduler<ValueType>::computeActionSupport(std::vector<uint_fast64_t> const& nondeterministicChoiceIndices) const {
            auto nrActions = nondeterministicChoiceIndices.back();
            storm::storage::BitVector result(nrActions);

            for (auto const& choicesPerMemoryNode : schedulerChoices) {

                STORM_LOG_ASSERT(nondeterministicChoiceIndices.size()-2 < choicesPerMemoryNode.size(), "Illegal model state index");
                for (uint64_t stateId = 0; stateId < nondeterministicChoiceIndices.size()-1; ++stateId) {
                    for (auto const& schedChoice : choicesPerMemoryNode[stateId].getChoiceAsDistribution()) {
                        STORM_LOG_ASSERT(schedChoice.first < nondeterministicChoiceIndices[stateId+1] - nondeterministicChoiceIndices[stateId], "Scheduler chooses action indexed " << schedChoice.first << " in state id "  << stateId << " but state contains only " << nondeterministicChoiceIndices[stateId+1] - nondeterministicChoiceIndices[stateId] << " choices .");
                        result.set(nondeterministicChoiceIndices[stateId] + schedChoice.first);
                    }
                }
            }
            return result;
        }

        template <typename ValueType>
        bool Scheduler<ValueType>::isPartialScheduler() const {
            return numOfUndefinedChoices != 0;
        }

        template <typename ValueType>
        bool Scheduler<ValueType>::isDeterministicScheduler() const {
            return numOfDeterministicChoices == (schedulerChoices.size() * schedulerChoices.begin()->size()) - numOfUndefinedChoices;
        }

        template <typename ValueType>
        bool Scheduler<ValueType>::isMemorylessScheduler() const {
            return getNumberOfMemoryStates() == 1;
        }

        template <typename ValueType>
        uint_fast64_t Scheduler<ValueType>::getNumberOfMemoryStates() const {
            return memoryStructure ? memoryStructure->getNumberOfStates() : 1;
        }

        template <typename ValueType>
        boost::optional<storm::storage::MemoryStructure> const& Scheduler<ValueType>::getMemoryStructure() const {
            return memoryStructure;
        }

        template <typename ValueType>
        void Scheduler<ValueType>::printToStream(std::ostream& out, std::shared_ptr<storm::models::sparse::Model<ValueType>> model, bool skipUniqueChoices) const {
            STORM_LOG_THROW(model == nullptr || model->getNumberOfStates() == schedulerChoices.front().size(), storm::exceptions::InvalidOperationException, "The given model is not compatible with this scheduler.");

            bool const stateValuationsGiven = model != nullptr && model->hasStateValuations();
            bool const choiceLabelsGiven = model != nullptr && model->hasChoiceLabeling();
            bool const choiceOriginsGiven = model != nullptr && model->hasChoiceOrigins();
            uint_fast64_t widthOfStates = std::to_string(schedulerChoices.front().size()).length();
            if (stateValuationsGiven) {
                widthOfStates += model->getStateValuations().getStateInfo(schedulerChoices.front().size() - 1).length() + 5;
            }
            widthOfStates = std::max(widthOfStates, (uint_fast64_t)12);
            uint_fast64_t numOfSkippedStatesWithUniqueChoice = 0;

            out << "___________________________________________________________________" << std::endl;
            out << (isPartialScheduler() ? "Partially" : "Fully") << " defined ";
            out << (isMemorylessScheduler() ? "memoryless " : "");
            out << (isDeterministicScheduler() ? "deterministic" : "randomized") << " scheduler";
            if (!isMemorylessScheduler()) {
                out << " with " << getNumberOfMemoryStates() << " memory states";
            }
            out << ":" << std::endl;
            STORM_LOG_WARN_COND(!(skipUniqueChoices && model == nullptr), "Can not skip unique choices if the model is not given.");
            out << std::setw(widthOfStates) << "model state:" << "    " << (isMemorylessScheduler() ? "" : " memory:     ") << "choice(s)" << std::endl;
            for (uint_fast64_t state = 0; state < schedulerChoices.front().size(); ++state) {
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
                for (uint_fast64_t memoryState = 0; memoryState < getNumberOfMemoryStates(); ++memoryState) {
                    // Indent if this is not the first memory state
                    if (firstMemoryState) {
                        firstMemoryState = false;
                    } else {
                        stateString << std::setw(widthOfStates) << "";
                        stateString << "    ";
                    }
                    // Print the memory state info
                    if (!isMemorylessScheduler()) {
                        stateString << "m" << std::setw(8) << memoryState;
                    }

                    // Print choice info
                    SchedulerChoice<ValueType> const& choice = schedulerChoices[memoryState][state];
                    if (choice.isDefined()) {
                        if (choice.isDeterministic()) {
                            if (choiceOriginsGiven) {
                                stateString << model->getChoiceOrigins()->getChoiceInfo(model->getTransitionMatrix().getRowGroupIndices()[state] + choice.getDeterministicChoice());
                            } else {
                                stateString << choice.getDeterministicChoice();
                            }
                            if (choiceLabelsGiven) {
                                auto choiceLabels = model->getChoiceLabeling().getLabelsOfChoice(model->getTransitionMatrix().getRowGroupIndices()[state] + choice.getDeterministicChoice());
                                stateString << " {" << boost::join(choiceLabels, ", ") << "}";
                            }
                        } else {
                            bool firstChoice = true;
                            for (auto const& choiceProbPair : choice.getChoiceAsDistribution()) {
                                if (firstChoice) {
                                    firstChoice = false;
                                } else {
                                    stateString << "   +    ";
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
                        }
                    } else {
                        if(!printUndefinedChoices) continue;
                        stateString << "undefined.";
                    }

                    if(!isMemorylessScheduler()) {
                        stateString << "    ";
                        for (auto const& choiceProbPair : choice.getChoiceAsDistribution()) {
                            for (auto entryIt = model->getTransitionMatrix().getRow(state + choiceProbPair.first).begin(); entryIt < model->getTransitionMatrix().getRow(state +  choiceProbPair.first).end(); ++entryIt) {
                                out << ", model state' = " << entryIt->getColumn() << ": (transition = " << state+choiceProbPair.first << ") -> " << "(m' = "<<this->memoryStructure->getSuccessorMemoryState(memoryState, entryIt - model->getTransitionMatrix().begin()) <<")";
                            }

                        }

                    stateString << std::endl;
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

            // TODO only for tests:
            if(!isMemorylessScheduler()) {
                out << memoryStructure->toString();
                out << std::endl;
            }
        }

        template <>
        void Scheduler<float>::printJsonToStream(std::ostream& out, std::shared_ptr<storm::models::sparse::Model<float>> model, bool skipUniqueChoices) const {
            STORM_LOG_THROW(isMemorylessScheduler(), storm::exceptions::NotImplementedException, "Json export of schedulers not implemented for this value type.");
        }

        template <typename ValueType>
        void Scheduler<ValueType>::printJsonToStream(std::ostream& out, std::shared_ptr<storm::models::sparse::Model<ValueType>> model, bool skipUniqueChoices) const {
            STORM_LOG_THROW(model == nullptr || model->getNumberOfStates() == schedulerChoices.front().size(), storm::exceptions::InvalidOperationException, "The given model is not compatible with this scheduler.");
            STORM_LOG_WARN_COND(!(skipUniqueChoices && model == nullptr), "Can not skip unique choices if the model is not given.");
            STORM_LOG_THROW(isMemorylessScheduler(), storm::exceptions::NotImplementedException, "Json export of schedulers with memory not implemented.");
            storm::json<storm::RationalNumber> output;
            for (uint64_t state = 0; state < schedulerChoices.front().size(); ++state) {
                // Check whether the state is skipped
                if (skipUniqueChoices && model != nullptr && model->getTransitionMatrix().getRowGroupSize(state) == 1) {
                    continue;
                }
                storm::json<storm::RationalNumber> stateChoicesJson;
                if (model && model->hasStateValuations()) {
                    stateChoicesJson["s"] = model->getStateValuations().template toJson<storm::RationalNumber>(state);
                } else {
                    stateChoicesJson["s"] = state;
                }
                auto const& choice = schedulerChoices.front()[state];
                storm::json<storm::RationalNumber> choicesJson;
                if (choice.isDefined()) {
                    for (auto const& choiceProbPair : choice.getChoiceAsDistribution()) {
                        uint64_t globalChoiceIndex = model->getTransitionMatrix().getRowGroupIndices()[state] + choiceProbPair.first;
                        storm::json<storm::RationalNumber> choiceJson;
                        if (model && model->hasChoiceOrigins() && model->getChoiceOrigins()->getIdentifier(globalChoiceIndex) != model->getChoiceOrigins()->getIdentifierForChoicesWithNoOrigin()) {
                            choiceJson["origin"] = model->getChoiceOrigins()->getChoiceAsJson(globalChoiceIndex);
                        }
                        if (model && model->hasChoiceLabeling()) {
                            auto choiceLabels = model->getChoiceLabeling().getLabelsOfChoice(globalChoiceIndex);
                            choiceJson["labels"] = std::vector<std::string>(choiceLabels.begin(), choiceLabels.end());
                        }
                        choiceJson["index"] = globalChoiceIndex;
                        choiceJson["prob"] = storm::utility::convertNumber<storm::RationalNumber>(choiceProbPair.second);
                        choicesJson.push_back(std::move(choiceJson));
                    }
                } else {
                    choicesJson = "undefined";
                }
                stateChoicesJson["c"] = std::move(choicesJson);
                output.push_back(std::move(stateChoicesJson));
            }
            out << output.dump(4);
        }

        template <typename ValueType>
        void Scheduler<ValueType>::setPrintUndefinedChoices(bool value) {
            printUndefinedChoices = value;
        }

        template class Scheduler<double>;
        template class Scheduler<float>;
        template class Scheduler<storm::RationalNumber>;
        template class Scheduler<storm::RationalFunction>;

    }
}
