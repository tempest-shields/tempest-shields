#include "storm/abstraction/prism/GameBddResult.h"

namespace storm {
    namespace abstraction {
        namespace prism {
         
            template <storm::dd::DdType DdType>
            GameBddResult<DdType>::GameBddResult() : bdd(), numberOfPlayer2Variables(0) {
                // Intentionally left empty.
            }
            
            template <storm::dd::DdType DdType>
            GameBddResult<DdType>::GameBddResult(storm::dd::Bdd<DdType> const& gameBdd, uint_fast64_t numberOfPlayer2Variables) : bdd(gameBdd), numberOfPlayer2Variables(numberOfPlayer2Variables) {
                // Intentionally left empty.
            }
         
            template class GameBddResult<storm::dd::DdType::CUDD>;
            template class GameBddResult<storm::dd::DdType::Sylvan>;
        }
    }
}