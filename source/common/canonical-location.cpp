
// Project includes
#include "misra-tidy/common/canonical-location.hpp"

// Clang includes
#include <clang/Basic/SourceManager.h>

// Standard includes
#include <utility>

namespace tidy {
    CanonicalLocation::CanonicalLocation(
        const clang::SourceLocation& location,
        const clang::SourceManager& sourceManager) {
        const auto decomposed = sourceManager.getDecomposedLoc(location);
        file = sourceManager.getFileEntryForID(decomposed.first);
        offset = decomposed.second;
    }

    bool CanonicalLocation::operator==(const CanonicalLocation& other) const
        noexcept {
        return this->file == other.file && this->offset == other.offset;
    }

    bool CanonicalLocation::operator!=(const CanonicalLocation& other) const
        noexcept {
        return !(*this == other);
    }
}  // namespace tidy
