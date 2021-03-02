#pragma once

#include <vector>
#include <memory>
#include "storm/storage/Scheduler.h"

namespace storm {
    namespace storage {
        class BitVector;
    }

    namespace modelchecker {
        namespace helper {
            template<typename ValueType>
            struct SMGSparseModelCheckingHelperReturnType {

                SMGSparseModelCheckingHelperReturnType(SMGSparseModelCheckingHelperReturnType const&) = delete;
                SMGSparseModelCheckingHelperReturnType(SMGSparseModelCheckingHelperReturnType&&) = default;

                SMGSparseModelCheckingHelperReturnType(std::vector<ValueType>&& values, storm::storage::BitVector& relevantStates, std::unique_ptr<storm::storage::Scheduler<ValueType>>&& scheduler = nullptr, std::vector<ValueType>&& choiceValues = nullptr) : values(std::move(values)), relevantStates(relevantStates), scheduler(std::move(scheduler)), choiceValues(std::move(choiceValues)) {
                    // Intentionally left empty.
                }

                virtual ~SMGSparseModelCheckingHelperReturnType() {
                    // Intentionally left empty.
                }

                // The values computed for the states.
                std::vector<ValueType> values;

                // The relevant states for which choice values have been computed.
                storm::storage::BitVector relevantStates;

                // A scheduler, if it was computed.
                std::unique_ptr<storm::storage::Scheduler<ValueType>> scheduler;

                // The values computed for the available choices.
                std::vector<ValueType> choiceValues;
            };
        }

    }
}
