#pragma once

#include "storm/utility/constants.h"

namespace storm {
    namespace storage {

        template <typename ValueType>
        class PreSchedulerChoice {

        public:

            /*!
             * Creates an undefined scheduler choice
             */
            PreSchedulerChoice();

            /*
             *
             */
            void addChoice(uint_fast64_t choiceIndex, ValueType probToSatisfy);

            /*
             *
             */
            bool isEmpty() const;

            /*
             *
             */
            std::vector<std::tuple<ValueType, uint_fast64_t>> const& getChoiceMap() const;

        private:
            // For now we only consider shields with deterministic choices.
            //std::map<ValueType, storm::storage::Distribution<ValueType, uint_fast64_t>> choiceMap;
            std::vector<std::tuple<ValueType, uint_fast64_t>> choiceMap;
        };

        template<typename ValueType>
        std::ostream& operator<<(std::ostream& out, PreSchedulerChoice<ValueType> const& schedulerChoice);
    }
}
