#ifndef MACRO_EXPAND_PPACTION_HPP
#define MACRO_EXPAND_PPACTION_HPP

// Project includes
#include "misra-tidy/common/location.hpp"

// Clang includes
#include <clang/Basic/SourceLocation.h>
#include "clang/Frontend/FrontendActions.h"

// Standard includes
#include <memory>
#include <string>

namespace clang {
class CompilerInstance;
class ASTConsumer;
}

namespace llvm {
class StringRef;
}

namespace tidy {
struct Query;
}

namespace tidy {
namespace MacroExpand {

/// \ingroup MacroExpand
///
/// The `MacroExpand::Action` class has a major responsibility at the very
/// beginning of the entire `macro-expand` tool.
///
/// It installs preprocessor callbacks to facilitate the part of macro-expand
/// dealing with macros.
class Action : public clang::PreprocessOnlyAction {
 public:
  using super = clang::ASTFrontendAction;
  using ASTConsumerPointer = std::unique_ptr<clang::ASTConsumer>;

  /// Constructor, taking the location at which to look for a function call and
  /// the ongoing `Query` object.
  Action(Query& query) : _query(query) {}

  /// Called before any file is even touched. Allows us to register a rewriter.
  bool BeginInvocation(clang::CompilerInstance& Compiler) override;

  /// Attempts to translate the `targetLocation` to a `clang::SourceLocation`
  /// and install preprocessor hooks for macros.
  bool BeginSourceFileAction(clang::CompilerInstance& compiler,
                             llvm::StringRef filename) override;

  void EndSourceFileAction() override;

 private:
  /// The ongoing `Query` object.
  Query& _query;
};

}  // namespace MacroExpand
}  // namespace tidy

#endif  // MACRO_EXPAND_PPACTION_HPP
