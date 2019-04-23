
// Project includes
#include "misra-tidy/common/definition-data.hpp"

// Third party includes
#include <third-party/json.hpp>


namespace tidy {
    nlohmann::json DefinitionData::toJson() const {
        // clang-format off
        nlohmann::json json = {
          {"location", location.toJson()},
          {"macro", isMacro}
        };
        // clang-format on

        if (!original.empty()) {
            json["text"] = original;
        }

        if (!rewritten.empty()) {
            json["rewritten"] = rewritten;
        }

        return json;
    }

}  // namespace tidy
