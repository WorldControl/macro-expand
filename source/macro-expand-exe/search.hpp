
#ifndef MACRO_EXPAND_SEARCH_HPP
#define MACRO_EXPAND_SEARCH_HPP

/// \defgroup MacroExpand

// Project includes
#include "misra-tidy/common/location.hpp"

// Standard includes
#include <string>
#include <vector>

namespace clang {
    namespace tooling {
        class CompilationDatabase;
    }
}

namespace tidy {
    struct Query;
    struct Result;
    struct Options;
    class Search {
    public:
        using CompilationDatabase = clang::tooling::CompilationDatabase;
        using SourceVector = std::vector<std::string>;

        /// Constructs a new `Search` object with a vector of all the files being searched
        Search(SourceVector& files);

        /// Runs the search on the given sources and with the given options.
        /// \returns A `Result`, ready to be printed to the console.
        Result run(CompilationDatabase& compilationDatabase,
            const Options& options);

    private:
        /// Performs the symbol search (& expand) phase. Decorates the `Query` with
        /// `DeclarationData` and `CallData`, as well as possibly `DefinitionData`.
        void _callsiteExpand(CompilationDatabase& compilationDatabase,  Query& query);
        /// Performs the header cleanup phase.
        void _cleanHeaderFiles(Query& query);
        SourceVector& _sourcelist;
    };
}  // namespace tidy

#endif  // MACRO_EXPAND_SEARCH_HPP
