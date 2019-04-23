
#ifndef TIDY_UTILS_COMMON_DEFINITION_DATA_HPP
#define TIDY_UTILS_COMMON_DEFINITION_DATA_HPP

// Project includes
#include "misra-tidy/common/location.hpp"

// Third party includes
#include <third-party/json.hpp>

// Standard includes
#include <string>

namespace clang {
class FunctionDecl;
class ASTContext;
}

namespace tidy {
struct Query;

/// Stores data about the definition of a function.
struct DefinitionData {

    DefinitionData(Location aLocation, std::string aOrg, std::string aReWr, bool aIsMacro=false)
        :location{ aLocation }
        , original{ aOrg }
        , rewritten{ aReWr }
        , isMacro{ aIsMacro }
    {}

  /// Converts the `DefinitionData` to JSON.
  nlohmann::json toJson() const;

  /// The `Location` of the definition in the source.
  Location location;

  /// The original, untouched source text of the definition.
  std::string original;

  /// The rewritten (expanded) source text of the definition.
  std::string rewritten;

  /// Whether this definition is from a macro or a real function.
  bool isMacro{false};
};
}  // namespace tidy

#endif  // TIDY_UTILS_COMMON_DEFINITION_DATA_HPP
