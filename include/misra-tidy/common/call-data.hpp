

#ifndef TIDY_UTILS_COMMON_CALL_DATA_HPP
#define TIDY_UTILS_COMMON_CALL_DATA_HPP

// Project includes
#include "misra-tidy/common/assignee-data.hpp"
#include "misra-tidy/common/range.hpp"

// LLVM includes
#include <llvm/ADT/Optional.h>

// Standard includes
#include <string>

namespace tidy {

/// Stores information about a function call.
struct CallData {
  /// Constructs the `CallData` with the extent of the call, leaving the `base`
  /// string empty and the `assignee` data null.
  explicit CallData(Range&& extent_);

  /// Constructs the `CallData` with an `assignee` and the extent of the call.
  CallData(AssigneeData&& assignee, Range&& extent_);

  /// Utility function to check if declaring the type of the assignee of the
  /// function return value is required. This is the case if there is an
  /// assignee at all and we picked up the type of the assignee (which would not
  /// be the case for *assignments* as opposed to constructions).
  bool requiresDeclaration() const noexcept;

  /// Any base expression to the function call, e.g. the `object` in
  /// `object.method()`.
  std::string base;

  /// Information about the assignee of the call expression, including things
  /// like the type and name of the variable as well as the assignment operator
  /// (we also refer to compound operators like `+=` to assignments in this
  /// case).
  llvm::Optional<AssigneeData> assignee;

  /// The source range of the entire call expression, from the first character
  /// of any variable declaration to the final semicolon. The semicolon is
  /// included in the range.
  Range extent;
};
}  // namespace tidy

#endif  // TIDY_UTILS_COMMON_CALL_DATA_HPP
