#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include "storm/storage/SchedulerChoice.h"
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
        class OptimalScheduler : public Scheduler<ValueType> {
        public:
            typedef uint_fast64_t OldChoice;
            /*!
             * Initializes a scheduler for the given number of model states.
             *
             * @param numberOfModelStates number of model states
             * @param memoryStructure the considered memory structure. If not given, the scheduler is considered as memoryless.
             */
            OptimalScheduler(uint_fast64_t numberOfModelStates, boost::optional<storm::storage::MemoryStructure> const& memoryStructure = boost::none);
            OptimalScheduler(uint_fast64_t numberOfModelStates, boost::optional<storm::storage::MemoryStructure>&& memoryStructure);

            /*!
             * Prints the scheduler to the given output stream.
             * @param out The output stream
             * @param model If given, provides additional information for printing (e.g., displaying the state valuations instead of state indices)
             * @param skipUniqueChoices If true, the (unique) choice for deterministic states (i.e., states with only one enabled choice) is not printed explicitly.
             *                          Requires a model to be given.
             */
            void printToStream(std::ostream& out, std::shared_ptr<storm::logic::ShieldExpression const> shieldingExpression, std::shared_ptr<storm::models::sparse::Model<ValueType>> model = nullptr, bool skipUniqueChoices = false) const;
        };
    }
}
