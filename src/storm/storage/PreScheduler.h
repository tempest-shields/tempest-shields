#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include "storm/storage/PreSchedulerChoice.h"
#include "storm/storage/Scheduler.h"
#include "storm/logic/ShieldExpression.h"

namespace storm {
    namespace storage {

        /*
         * TODO needs obvious changes in all comment blocks
         */
        template <typename ValueType>
        class PreScheduler {
        public:

            /*!
             * Initializes a scheduler for the given number of model states.
             *
             * @param numberOfModelStates number of model states
             * @param memoryStructure the considered memory structure. If not given, the scheduler is considered as memoryless.
             */
            PreScheduler(uint_fast64_t numberOfModelStates, boost::optional<storm::storage::MemoryStructure> const& memoryStructure = boost::none);
            PreScheduler(uint_fast64_t numberOfModelStates, boost::optional<storm::storage::MemoryStructure>&& memoryStructure);

            bool isMemorylessScheduler() const;
            uint_fast64_t getNumberOfMemoryStates() const;

            void setChoice(PreSchedulerChoice<ValueType> const& choice, uint_fast64_t modelState, uint_fast64_t memoryState);

            /*!
             * Prints the scheduler to the given output stream.
             */
            void printToStream(std::ostream& out, std::shared_ptr<storm::logic::ShieldExpression const> shieldingExpression, std::shared_ptr<storm::models::sparse::Model<ValueType>> model = nullptr, bool skipUniqueChoices = false) const;

        private:
            boost::optional<storm::storage::MemoryStructure> memoryStructure;
            std::vector<std::vector<PreSchedulerChoice<ValueType>>> schedulerChoices;

            bool printUndefinedChoices = false;

            uint_fast64_t numOfUndefinedChoices;
            uint_fast64_t numOfDeterministicChoices;
            uint_fast64_t numOfDontCareStates;
        };
    }
}
