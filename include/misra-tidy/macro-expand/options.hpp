#ifndef MACRO_EXPAND_OPTIONS_HPP
#define MACRO_EXPAND_OPTIONS_HPP

namespace tidy {
    /// Options for a query.
    struct Options {
        /// Whether to expand function-macro calls
        bool wantsFcnCallExpand;

        /// Whether to expand object-macro calls
        bool wantsObjectExpand;

        /// Whether to remove unused macro definitions in non-system source files
        bool wantsUnusedRemoved;

        /// Whether to include the rewritten function body information for the
        /// function.
        bool wantsRewritten;
    };
}  // namespace tidy

#endif  // MACRO_EXPAND_OPTIONS_HPP
