#include "storm/storage/PreSchedulerChoice.h"

#include "storm/utility/constants.h"
#include "storm/utility/macros.h"

#include "storm/exceptions/InvalidOperationException.h"
#include "storm/adapters/RationalFunctionAdapter.h"
#include "storm/adapters/RationalNumberAdapter.h"

namespace storm {
    namespace storage {

        template <typename ValueType>
        PreSchedulerChoice<ValueType>::PreSchedulerChoice() {
            // Intentionally left empty
        }

        template <typename ValueType>
        void PreSchedulerChoice<ValueType>::addChoice(uint_fast64_t choiceIndex, ValueType probToSatisfy) {
            choiceMap.emplace_back(probToSatisfy, choiceIndex);
        }

        template <typename ValueType>
        std::vector<std::tuple<ValueType, uint_fast64_t>> const& PreSchedulerChoice<ValueType>::getChoiceMap() const {
            return choiceMap;
        }

        template <typename ValueType>
        bool PreSchedulerChoice<ValueType>::isEmpty() const {
            return choiceMap.size() == 0;
        }

        template <typename ValueType>
        std::ostream& operator<<(std::ostream& out, PreSchedulerChoice<ValueType> const& schedulerChoice) {
            out << schedulerChoice.getChoiceMap().size();
            if (!schedulerChoice.isEmpty()) {
                for(auto const& choice : schedulerChoice.getChoiceMap()) {
                    out << std::get<0>(choice) << ": " << std::get<1>(choice);
                }
            } else {
              out << "undefined";
            }
            return out;
        }

        template class PreSchedulerChoice<double>;
        template std::ostream& operator<<(std::ostream& out, PreSchedulerChoice<double> const& schedulerChoice);
        template class PreSchedulerChoice<float>;
        template std::ostream& operator<<(std::ostream& out, PreSchedulerChoice<float> const& schedulerChoice);
        template class PreSchedulerChoice<storm::RationalNumber>;
        template std::ostream& operator<<(std::ostream& out, PreSchedulerChoice<storm::RationalNumber> const& schedulerChoice);
        template class PreSchedulerChoice<storm::RationalFunction>;
        template std::ostream& operator<<(std::ostream& out, PreSchedulerChoice<storm::RationalFunction> const& schedulerChoice);

    }
}
