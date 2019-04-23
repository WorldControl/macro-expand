
#ifndef MACRO_EXPAND_PPPREPROCESSOR_HOOKS_HPP
#define MACRO_EXPAND_PPPREPROCESSOR_HOOKS_HPP

// Clang includes
#include <clang/Basic/SourceLocation.h>
#include <clang/Lex/PPCallbacks.h>

// LLVM includes
#include <llvm/ADT/StringMap.h>

// Standard includes
#include <iosfwd>
#include <unordered_map>

namespace clang {
    class LangOptions;
    class CompilerInstance;
    class MacroArgs;
    class MacroDefinition;
    class MacroDirective;
    class MacroInfo;
    class Preprocessor;
    class SourceManager;
    class Token;
}  // namespace clang

namespace llvm {
    template <unsigned int InternalLen>
    class SmallString;
}

namespace tidy {
    struct Query;
}  // namespace tidy

namespace std {
    template<> struct hash<clang::SourceLocation>
    {
        typedef clang::SourceLocation argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const noexcept
        {
            return s.getRawEncoding(); // or use boost::hash_combine (see Discussion)
        }
    };
}

namespace tidy {
    namespace MacroExpand {

        /// Class responsible for inspecting macros during symbol search.
        ///
        /// For a given invocation `f(x)`, we don't know from the raw source text if `f`
        /// is a function or a macro. Also, at the point where we have the chance to
        /// hook  into the preprocessor (inside the `MacroExpand::Action`), we don't
        /// yet have an AST, so we cannot find this information out. As such, we need to
        /// hook into the preprocessing stage and look out for macro invocations. If
        /// there is one such invocation whose location matches the cursor, we have
        /// determined that the function call is actually a macro expansion and we can
        /// process it straight away into a `DefinitionData` object, since macros must
        /// always be defined on the spot. Since translation units are preprocessed
        /// anyway irrespective of whether or not we need something from this stage,
        /// this functionality incurs very little performance overhead.
        struct MacroSearch : public clang::PPCallbacks {
        public:
            /// Constructor.
            MacroSearch(clang::CompilerInstance& compiler,
                Query& query);

            /// Hook for any macro expansion. A macro expansion will either be a
            /// function-macro call like `f(x)`, or simply an object-macro expansion like
            /// `NULL` (which is `(void*)0`).
            void MacroExpands(const clang::Token& macroNameToken,
                const clang::MacroDefinition& macroDefinition,
                clang::SourceRange range,
                const clang::MacroArgs* macroArgs) override;

            /// Hook for any macro definition. We essentially go through all macro definitions 
            /// and record where the definitions are.
            void MacroDefined(const clang::Token & macroNameTok,
                const clang::MacroDirective *macroDirective) override;

            /// Hook for any macro un-definition. We essentially go through all macro un-definitions 
            /// and record where the un-definitions are.
            /// \note: There's an incompatibility in this function signature in Clang 5
            void MacroUndefined(const clang::Token &macroNameTok, 
                const clang::MacroDefinition &undef) override;

            void EndOfMainFile() override;

        private:
            using ParameterMap = llvm::StringMap<llvm::SmallString<32>>;

            /// Rewrites a function-macro contents using the arguments it was invoked
            /// with. This function identifies `#` and `##` stringification and
            /// concatenation operators and deals with them correctly.
            std::string _rewriteMacro(const clang::MacroInfo& info,
                const ParameterMap& mapping);

            /// Creates a mapping from parameter names to argument expressions.
            ParameterMap _createParameterMap(const clang::MacroInfo& info,
                const clang::MacroArgs& arguments);

            /// Gets the spelling (string representation) of a token using the
            /// preprocessor.
            std::string _getSpelling(const clang::Token& token) const;  // NOLINT

            /// The current `clang::SourceManager` from the compiler.
            clang::SourceManager& _sourceManager;

            /// The current `clang::LangOptions` from the compiler.
            const clang::LangOptions& _languageOptions;

            /// The `clang::Preprocessor` instance we operate on.
            clang::Preprocessor& _preprocessor;

            /// The ongoing `Query` object.
            Query& _query;

            struct MacroContext {
                const clang::MacroInfo &_defMacro;
                llvm::Optional<const clang::SourceRange> _undefRange;
                size_t _count = 0;
            };
            std::unordered_map<clang::SourceLocation, MacroContext> _defCountMap;
        };

    }  // namespace MacroExpand
}  // namespace tidy

#endif  // MACRO_EXPAND_PPPREPROCESSOR_HOOKS_HPP
