
// Project includes
#include "misra-tidy/common/offset.hpp"
#include "misra-tidy/common/routines.hpp"
#include "misra-tidy/macro-expand/query.hpp"
#include "misra-tidy/macro-expand/action.hpp"
#include "misra-tidy/macro-expand/macro-search.hpp"

// Clang includes
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TokenKinds.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/Token.h>

// LLVM includes
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/ADT/Twine.h>

// Standard includes
#include <cassert>
#include <memory>
#include <string>
#include <utility>


namespace tidy {
    namespace MacroExpand {

        bool Action::BeginInvocation(clang::CompilerInstance& Compiler) {
            _query._rewriter.emplace( Compiler.getSourceManager(), Compiler.getLangOpts() );
            return true;
        }

        bool Action::BeginSourceFileAction(clang::CompilerInstance& compiler, llvm::StringRef) {
            /// Given a `clang::CompilerInstance`, installs appropriate preprocessor
            /// hooks for macro search (looking for macros with the name of the target
            /// function) with the `CompilerInstance`.
            auto hooks = std::make_unique<MacroSearch>(compiler, _query);
            compiler.getPreprocessor().SetSuppressIncludeNotFoundError(true);
            compiler.getPreprocessor().addPPCallbacks(std::move(hooks));

            /// Continue.
            return true;
        }

        void Action::EndSourceFileAction() {
            if (_query.options.wantsRewritten && _query._rewriter)
                _query._rewriter->overwriteChangedFiles();
        }

    }  // namespace MacroExpand
}  // namespace tidy
