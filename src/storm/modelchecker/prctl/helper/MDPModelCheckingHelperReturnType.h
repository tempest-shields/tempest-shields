#ifndef MDPMODELCHECKINGHELPERRETURNTYPE_H
#define	MDPMODELCHECKINGHELPERRETURNTYPE_H

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
            struct MDPSparseModelCheckingHelperReturnType {

                MDPSparseModelCheckingHelperReturnType(MDPSparseModelCheckingHelperReturnType const&) = delete;
                MDPSparseModelCheckingHelperReturnType(MDPSparseModelCheckingHelperReturnType&&) = default;

                MDPSparseModelCheckingHelperReturnType(std::vector<ValueType>&& values, storm::storage::BitVector&& maybeStates = nullptr, std::unique_ptr<storm::storage::Scheduler<ValueType>>&& scheduler = nullptr, std::vector<ValueType>&& choiceValues = nullptr) : values(std::move(values)), maybeStates(maybeStates), scheduler(std::move(scheduler)), choiceValues(std::move(choiceValues)) {
                    // Intentionally left empty.
                }

                virtual ~MDPSparseModelCheckingHelperReturnType() {
                    // Intentionally left empty.
                }

                // The values computed for the states.
                std::vector<ValueType> values;

                // The maybe states of the model
                storm::storage::BitVector maybeStates;

                // A scheduler, if it was computed.
                std::unique_ptr<storm::storage::Scheduler<ValueType>> scheduler;

                // The values computed for the available choices.
                std::vector<ValueType> choiceValues;
            };
        }

    }
}

#endif	/* MDPMODELCHECKINGRETURNTYPE_H */
