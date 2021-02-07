#include "storm/logic/PlayerCoalition.h"

#include <iostream>

namespace storm {
    namespace logic {

        PlayerCoalition::PlayerCoalition(std::vector<boost::variant<std::string, storm::storage::PlayerIndex>> playerIds) : _playerIds(playerIds) {
            // Intentionally left empty.
        }

        std::vector<boost::variant<std::string, storm::storage::PlayerIndex>> const& PlayerCoalition::getPlayers() const {
            return _playerIds;
        }

        std::ostream& operator<<(std::ostream& out, PlayerCoalition const& coalition) {
            bool firstItem = true;
            for (auto const& player : coalition._playerIds) {
                if(firstItem) { firstItem = false; } else { out << ","; }
                if (player.type() == typeid(std::string)) {
                    out << boost::get<std::string>(player);
                } else {
                    out << boost::get<storm::storage::PlayerIndex>(player);
                }
            }
            return out;
        }
    }
}
