

// Project includes
#include "misra-tidy/common/range.hpp"

// Third party includes
#include <third-party/json.hpp>

// Clang includes
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>

namespace tidy {
    Range::Range(const clang::SourceRange& range,
        const clang::SourceManager& sourceManager)
        : begin(range.getBegin(), sourceManager)
        , end(range.getEnd(), sourceManager)
        , filename(sourceManager.getFilename(range.getBegin())) {
    }

    nlohmann::json Range::toJson() const {
        // clang-format off
        return {
            {"filename", filename },
            { "begin", begin.toJson() },
            { "end", end.toJson() }
        };
        // clang-format on
    }

}  // namespace tidy
