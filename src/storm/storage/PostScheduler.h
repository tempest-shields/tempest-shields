#pragma once

#include <cstdint>
#include <map>
#include "storm/storage/PostSchedulerChoice.h"
#include "storm/storage/Scheduler.h"
#include "storm/logic/ShieldExpression.h"

namespace storm {
    namespace storage {

        /*
         * TODO needs obvious changes in all comment blocks
         * This class defines which action is chosen in a particular state of a non-deterministic model. More concretely, a scheduler maps a state s to i
         * if the scheduler takes the i-th action available in s (i.e. the choices are relative to the states).
         * A Choice can be undefined, deterministic
         */
        template <typename ValueType>
        class PostScheduler {
        public:
            typedef uint_fast64_t OldChoice;
            /*!
             * Initializes a scheduler for the given number of model states.
             *
             * @param numberOfModelStates number of model states
             * @param memoryStructure the considered memory structure. If not given, the scheduler is considered as memoryless.
             */
            PostScheduler(uint_fast64_t numberOfModelStates, std::vector<uint_fast64_t> numberOfChoicesPerState, boost::optional<storm::storage::MemoryStructure> const& memoryStructure = boost::none);
            PostScheduler(uint_fast64_t numberOfModelStates, std::vector<uint_fast64_t> numberOfChoicesPerState, boost::optional<storm::storage::MemoryStructure>&& memoryStructure);

            /*!
             * Sets the choice defined by the scheduler for the given state.
             *
             * @param choice The choice to set for the given state.
             * @param modelState The state of the model for which to set the choice.
             * @param memoryState The state of the memoryStructure for which to set the choice.
             */
            void setChoice(PostSchedulerChoice<ValueType> const& newChoice, uint_fast64_t modelState, uint_fast64_t memoryState = 0);

            /*!
             * Is the scheduler defined on the states indicated by the selected-states bitvector?
             */
            bool isChoiceSelected(BitVector const& selectedStates, uint64_t memoryState = 0) const;

            /*!
             * Clears the choice defined by the scheduler for the given state.
             *
             * @param modelState The state of the model for which to clear the choice.
             * @param memoryState The state of the memoryStructure for which to clear the choice.
             */
            void clearChoice(uint_fast64_t modelState, uint_fast64_t memoryState = 0);

            /*!
             * Compute the Action Support: A bit vector that indicates all actions that are selected with positive probability in some memory state
             */
            //storm::storage::BitVector computeActionSupport(std::vector<uint64_t> const& nondeterministicChoiceIndicies) const;

            /*!
             * Retrieves whether all defined choices are deterministic
             */
            bool isDeterministicScheduler() const;

            /*!
             * Retrieves whether the scheduler considers a trivial memory structure (i.e., a memory structure with just a single state)
             */
            bool isMemorylessScheduler() const;

            /*!
             * Retrieves the number of memory states this scheduler considers.
             */
            //uint_fast64_t getNumberOfMemoryStates() const;

            /*!
             * Retrieves the memory structure associated with this scheduler
             */
            //boost::optional<storm::storage::MemoryStructure> const& getMemoryStructure() const;

            /*!
             * Prints the scheduler to the given output stream.
             * @param out The output stream
             * @param model If given, provides additional information for printing (e.g., displaying the state valuations instead of state indices)
             * @param skipUniqueChoices If true, the (unique) choice for deterministic states (i.e., states with only one enabled choice) is not printed explicitly.
             *                          Requires a model to be given.
             */
            void printToStream(std::ostream& out, std::shared_ptr<storm::logic::ShieldExpression const> shieldingExpression, std::shared_ptr<storm::models::sparse::Model<ValueType>> model = nullptr, bool skipUniqueChoices = false) const;
        private:
            boost::optional<storm::storage::MemoryStructure> memoryStructure;
            std::vector<std::vector<PostSchedulerChoice<ValueType>>> schedulerChoiceMapping;

            bool printUndefinedChoices = false;

            uint_fast64_t numOfUndefinedChoices;
            uint_fast64_t numOfDeterministicChoices;
            uint_fast64_t numOfDontCareStates;
            std::vector<uint_fast64_t> numberOfChoicesPerState;
            uint_fast64_t numberOfChoices;
        };
    }
}
