#include <vector>
#include <string>

namespace storm {
    namespace parser {
        /**
         * Given a string seperated by commas, returns the values.
         */
        std::vector<std::string> parseCommaSeperatedValues(std::string const& input);
        
    }
}