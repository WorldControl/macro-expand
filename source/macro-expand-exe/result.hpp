
#ifndef MACRO_EXPAND_RESULT_HPP
#define MACRO_EXPAND_RESULT_HPP

// Project includes
#include "misra-tidy/common/definition-data.hpp"
#include "misra-tidy/common/range.hpp"
#include "misra-tidy/macro-expand/query.hpp"

// LLVM includes
#include <llvm/ADT/Optional.h>

// Third party includes
#include <third-party/json.hpp>

namespace llvm {
class raw_ostream;
}

namespace tidy {
/// Stores the result of a `Query`.
///
/// Converting this structure to YAML gives the full (nested) output of
/// macro-expand, including the call range, declaration and definition
/// information.
struct Result {
  /// Constructs a `Result` from a completed `Query`.
  ///
  /// If the query's options specify that the call range is requested, the query
  /// must contain `CallData`.
  explicit Result(Query&& query);

  /// Converts the `Result` to JSON.
  nlohmann::json toJson() const;

  std::vector<Query::IndividualMacroInfo> _macros;
  bool _needsJson;
};
}  // namespace tidy

#endif  // MACRO_EXPAND_RESULT_HPP
