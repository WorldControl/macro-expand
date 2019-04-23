// Project includes
#include "misra-tidy/common/routines.hpp"
#include "misra-tidy/macro-expand/action-factory.hpp"
#include "misra-tidy/macro-expand/query.hpp"
#include "result.hpp"
#include "search.hpp"

// Clang includes
#include <clang/Tooling/Tooling.h>

// LLVM includes
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>

// Standard includes
#include <cstdlib>
#include <string>
#include <type_traits>
#include <fstream>

namespace tidy {
    Search::Search(SourceVector& files)
        :_sourcelist(files){
        for (auto &file : _sourcelist)
            file = Routines::makeAbsolute(file);
    }

    Result Search::run(clang::tooling::CompilationDatabase& compilationDatabase,
        const Options& options) {
        Query query(options);

        _callsiteExpand(compilationDatabase, query);
        _cleanHeaderFiles(query);

        return Result(std::move(query));
    }

    void Search::_callsiteExpand(CompilationDatabase& compilationDatabase, Query& query) {
        clang::tooling::ClangTool MacroExpand(compilationDatabase, _sourcelist );
        tidy::MacroExpand::ActionFactory actionFactory(query);
        const auto error = MacroExpand.run( &actionFactory);
        if (error)
            throw Routines::ErrorCode{ "fatal error" };
    }

    void Search::_cleanHeaderFiles(Query& query) {
        if (!query.options.wantsUnusedRemoved)
            return;
        std::unordered_map<std::string, std::vector<size_t>> linesToDelete;
        for (const auto&it : query._macroDefinitionsInHeaders)
        {
            if (it.second.first == 0) {
                //delete those lines
                linesToDelete[it.first.filename].push_back(it.first.offset.line);
            }
        }
        for (auto&it : linesToDelete)
        {
            std::sort(it.second.begin(), it.second.end());
            auto last = std::unique(it.second.begin(), it.second.end());
            it.second.erase(last, it.second.end());
            std::ifstream fid;
            std::ostringstream ss;
            fid.open(it.first);
            int lineno = 0;
            std::string data;
            while (std::getline(fid, data))
            {
                ++lineno;
                if (std::find(it.second.begin(), it.second.end(), lineno) != it.second.end())
                    continue;
                ss << data << std::endl;
            }
            if (!fid.eof())
                break;
            fid.close();
            std::ofstream fid_out(it.first);
            fid_out << ss.str();
            assert(fid_out.good());
            fid_out.close();
        }
    }

}  // namespace tidy
