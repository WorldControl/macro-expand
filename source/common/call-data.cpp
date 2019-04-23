
// Project includes
#include "misra-tidy/common/call-data.hpp"

// LLVM includes
#include <llvm/ADT/Optional.h>

// Standard includes
#include <type_traits>

namespace tidy {
CallData::CallData(AssigneeData&& assignee, Range&& extent_)
: assignee(std::move(assignee)), extent(extent_) {
}

CallData::CallData(Range&& extent_) : extent(extent_) {
}

bool CallData::requiresDeclaration() const noexcept {
  return assignee.hasValue() && assignee->type.hasValue();
}

}  // namespace tidy
