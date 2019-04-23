// Project includes
#include "misra-tidy/common/routines.hpp"
#include "misra-tidy/macro-expand/options.hpp"
#include "result.hpp"
#include "search.hpp"

// Third-party includes
#include <third-party/json.hpp>

// Clang includes
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/CompilationDatabase.h>

// LLVM includes
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

// Standard includes
#include <string>
#include <vector>

namespace {
    llvm::cl::OptionCategory clangExpandCategory("macro-expand options");

    llvm::cl::extrahelp clangExpandCategoryHelp(R"(
Retrieves all macro definitions performs automatic parameter replacement. Allows for happy refactoring without
source file gymnastics.
)");

    llvm::cl::opt<bool> fcnCallExpansionOption("fcnExp",
        llvm::cl::init(true),
        llvm::cl::desc("Whether to replace function like macros. For example, \"#define USTR(a) U ## a\""),
        llvm::cl::cat(clangExpandCategory));

    llvm::cl::opt<bool> objectExpansionOption(
        "objExp",
        llvm::cl::init(true),
        llvm::cl::desc("Whether to replace object like macros. For example, \"#define PI 3.14182\""),
        llvm::cl::cat(clangExpandCategory));

    llvm::cl::opt<bool> removeUnusedMacrosOption(
        "remUnused",
        llvm::cl::init(true),
        llvm::cl::desc("Whether to remove unused macros that are defined in non-system source files"),
        llvm::cl::cat(clangExpandCategory));

    llvm::cl::opt<bool> rewriteOption(
        "rewrite",
        llvm::cl::init("ToJson"),
        llvm::cl::desc("Whether to generate the rewritten (expand) definition"),
        llvm::cl::cat(clangExpandCategory));

    llvm::cl::extrahelp
        commonHelp(clang::tooling::CommonOptionsParser::HelpMessage);
}  // namespace

auto main(int argc, const char* argv[]) -> int {
    using namespace clang::tooling;  // NOLINT(build/namespaces)

    CommonOptionsParser options(argc, argv, clangExpandCategory);
    auto sources = options.getSourcePathList();
    auto& db = options.getCompilations();

    try {
        // clang-format off
        tidy::Search search(sources);
        auto result = search.run(db, {
            fcnCallExpansionOption,
            objectExpansionOption,
            removeUnusedMacrosOption,
            rewriteOption
        });
        // clang-format on
        llvm::outs() << result.toJson().dump(2) << '\n';
    }
    catch (tidy::Routines::ErrorCode &er) {
        llvm::outs() << er.message;
        exit(EXIT_FAILURE);
    }
}
