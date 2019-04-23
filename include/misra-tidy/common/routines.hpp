
#ifndef TIDY_UTILS_COMMON_ROUTINES_HPP
#define TIDY_UTILS_COMMON_ROUTINES_HPP

// Standard includes
#include <string>

namespace clang {
    class SourceLocation;
    class SourceManager;
    class LangOptions;
    class SourceRange;
    class ASTContext;
}

namespace llvm {
    class Twine;
}

namespace tidy {
    namespace Routines {
        struct ErrorCode {
            std::string message;
        };
        bool locationsAreEqual(const clang::SourceLocation& first,
            const clang::SourceLocation& second,
            const clang::SourceManager& sourceManager);

        void assertTrowIfFail(bool condition, const std::string &message);

        /// Retrieves the raw source text within a range, as a string.
        std::string getSourceText(const clang::SourceRange& range,
            clang::SourceManager& sourceManager,
            const clang::LangOptions& languageOptions);

        /// Retrieves the raw source text within a range, as a string. Passes the source
        /// manager and language options from the `ASTContext` to the other overload.
        std::string getSourceText(const clang::SourceRange& range,
            clang::ASTContext& context);

        /// Turns a file path into an absolute file path.
        std::string makeAbsolute(const std::string& filename);

        /// Prints an error message to stderr and exits. the program.
        [[noreturn]] void error(const char* message);

        /// Prints an error message to stderr and exits. the program. The message is
        /// taken as the result of calling `twine.str()`.
        [[noreturn]] void error(llvm::Twine&& twine);

    }  // namespace Routines
}  // namespace tidy


#endif  // TIDY_UTILS_COMMON_ROUTINES_HPP
