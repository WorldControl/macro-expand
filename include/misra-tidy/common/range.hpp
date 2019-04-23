
#ifndef TIDY_UTILS_COMMON_RANGE_HPP
#define TIDY_UTILS_COMMON_RANGE_HPP

// Project includes
#include "misra-tidy/common/offset.hpp"

// Third party includes
#include <third-party/json.hpp>

namespace clang {
class SourceManager;
class SourceRange;
}

namespace tidy {
/// A range of source code, represented by a start and end offset.
struct Range {
  /// Constructs a range from a `clang::SourceRange` and `clang::SourceManager`,
  /// used to obtain the
  /// strat and end `Offset`s.
  Range(const clang::SourceRange& range,
        const clang::SourceManager& sourceManager);

  /// Converts the `Range` to JSON.
  nlohmann::json toJson() const;

  /// The starting offset.
  Offset begin;

  /// The ending offset. May be inclusive or exclusive depending on the context.
  Offset end;

  /// The name of the file this location is from.
  std::string filename;
};
}  // namespace tidy

#endif  // TIDY_UTILS_COMMON_RANGE_HPP
