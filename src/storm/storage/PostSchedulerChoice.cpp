#include "storm/storage/PostSchedulerChoice.h"

#include "storm/utility/constants.h"
#include "storm/utility/macros.h"

#include "storm/exceptions/InvalidOperationException.h"
#include "storm/adapters/RationalFunctionAdapter.h"
#include "storm/adapters/RationalNumberAdapter.h"

namespace storm {
    namespace storage {

        template <typename ValueType>
        PostSchedulerChoice<ValueType>::PostSchedulerChoice() {
            // Intentionally left empty
        }

        template <typename ValueType>
        void PostSchedulerChoice<ValueType>::addChoice(uint_fast64_t oldChoiceIndex, uint_fast64_t newChoiceIndex) {
            choiceMap.emplace_back(oldChoiceIndex, newChoiceIndex);
        }

        template <typename ValueType>
        std::vector<std::tuple<uint_fast64_t, uint_fast64_t>> const& PostSchedulerChoice<ValueType>::getChoiceMap() const {
            return choiceMap;
        }

        template <typename ValueType>
        std::tuple<uint_fast64_t, uint_fast64_t> const& PostSchedulerChoice<ValueType>::getChoice(uint_fast64_t choiceIndex) const {
            return choiceMap.at(choiceIndex);
        }

        template <typename ValueType>
        bool PostSchedulerChoice<ValueType>::isEmpty() const {
            return choiceMap.size() == 0;
        }

        template <typename ValueType>
        std::ostream& operator<<(std::ostream& out, PostSchedulerChoice<ValueType> const& schedulerChoice) {
            if (!schedulerChoice.isEmpty()) {
                bool firstChoice = true;
                for(auto const& choice : schedulerChoice.getChoiceMap()) {
                    if(firstChoice) firstChoice = false;
                    else out << ", ";
                    out << std::get<0>(choice) << " -> " << std::get<1>(choice);
                }
            } else {
              out << "undefined";
            }
            return out;
        }

        template class PostSchedulerChoice<double>;
        template std::ostream& operator<<(std::ostream& out, PostSchedulerChoice<double> const& schedulerChoice);
        template class PostSchedulerChoice<float>;
        template std::ostream& operator<<(std::ostream& out, PostSchedulerChoice<float> const& schedulerChoice);
        template class PostSchedulerChoice<storm::RationalNumber>;
        template std::ostream& operator<<(std::ostream& out, PostSchedulerChoice<storm::RationalNumber> const& schedulerChoice);
        template class PostSchedulerChoice<storm::RationalFunction>;
        template std::ostream& operator<<(std::ostream& out, PostSchedulerChoice<storm::RationalFunction> const& schedulerChoice);

    }
}
