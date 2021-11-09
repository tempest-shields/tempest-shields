#pragma once

#include "storm/utility/constants.h"

namespace storm {
    namespace storage {

        template <typename ValueType>
        class PostSchedulerChoice {

        public:

            /*!
             * Creates an undefined scheduler choice
             */
            PostSchedulerChoice();

            /*
             *
             */
            void addChoice(uint_fast64_t oldChoiceIndex, uint_fast64_t newChoiceIndex);

            /*
             *
             */
            bool isEmpty() const;

            /*
             *
             */
            std::vector<std::tuple<uint_fast64_t, uint_fast64_t>> const& getChoiceMap() const;

            /*
             *
             */
            std::tuple<uint_fast64_t, uint_fast64_t> const& getChoice(uint_fast64_t choiceIndex) const;

        private:
            //std::vector<std::tuple<uint_fast64_t, storm::storage::Distribution<ValueType, uint_fast64_t>>> choiceMap;
            std::vector<std::tuple<uint_fast64_t, uint_fast64_t>> choiceMap;
        };

        template<typename ValueType>
        std::ostream& operator<<(std::ostream& out, PostSchedulerChoice<ValueType> const& schedulerChoice);
    }
}
