
#ifndef TIDY_UTILS_COMMON_QUERY_HPP
#define TIDY_UTILS_COMMON_QUERY_HPP

// Project includes
#include "misra-tidy/common/call-data.hpp"
#include "misra-tidy/common/definition-data.hpp"
#include "misra-tidy/macro-expand/options.hpp"

// Clang includes
#include <clang/Rewrite/Core/Rewriter.h>

// LLVM includes
#include <llvm/ADT/Optional.h>

// Standard includes
#include <unordered_map>

namespace tidy {

/// Stores the options and state of an ongoing query.
///
/// A `Query` object is created inside `run()` and passed through all stages of
/// the tool to collect and store data. After the search has finished, the
/// `Query` can be converted to a `Result` and finally printed to the console.
struct Query {
  /// Constructs a fresh `Query` with the given `Options`.
  explicit Query(Options options_) : options(options_) {
  }
  struct IndividualMacroInfo {
      /// Possibly collected `CallData`.
      llvm::Optional<CallData> call;

      /// Possibly collected `DefinitionData`.
      llvm::Optional<DefinitionData> definition;
  };
  /// A list of every single macro invocation in the source file under consideration
  std::vector<IndividualMacroInfo> _macroInvocations;

  /// A count of how often every macro definition encountered in a non-system, writable 
  /// header was used
  using MacroDefCountMap = std::unordered_map<const Location, std::pair</*count*/size_t, /*undefLocation*/llvm::Optional<Location>>>;
  MacroDefCountMap _macroDefinitionsInHeaders;

  /// The `Options` of the query (i.e. what information the user wants).
  const Options options;
  
  /// A `clang::Rewriter` to rewrite source code. 
  llvm::Optional<clang::Rewriter> _rewriter;
private:
    Query(const Query&);          ///not copy constructible
    void operator=(const Query&); ///not copy assignable
};

}  // namespace tidy

#endif  // TIDY_UTILS_COMMON_QUERY_HPP
