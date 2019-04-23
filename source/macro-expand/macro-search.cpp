
// Project includes
#include "misra-tidy/common/call-data.hpp"
#include "misra-tidy/common/definition-data.hpp"
#include "misra-tidy/common/location.hpp"
#include "misra-tidy/common/range.hpp"
#include "misra-tidy/common/routines.hpp"
#include "misra-tidy/macro-expand/query.hpp"
#include "misra-tidy/macro-expand/macro-search.hpp"

// Clang includes
#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/TokenKinds.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/Token.h>
#include <clang/Lex/TokenLexer.h>
#include <clang/Rewrite/Core/Rewriter.h>

// LLVM includes
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>

// System includes
#include <cassert>
#include <iterator>
#include <string>
#include <type_traits>


namespace clang {
    class LangOptions;
}

namespace tidy {
    namespace MacroExpand {
        namespace {

            /// Gets the raw source text of a macro definition.
            std::string getDefinitionText(const clang::MacroInfo& info,
                clang::SourceManager& sourceManager,
                const clang::LangOptions& languageOptions) {
                // Using the rewriter (without actually rewriting) is honestly the only way I
                // found to get at the raw source text in a macro-safe way.
                const auto start = info.tokens_begin()->getLocation();
                const auto end = std::prev(info.tokens_end())->getEndLoc();
                return Routines::getSourceText({ start, end }, sourceManager, languageOptions);
            }

            /// Rewrites a macro argument use inside a macro in case the parameter it maps
            /// to was found to be preceded with a `#` stringification operator. It
            /// basically quotes it.
            void rewriteStringifiedMacroArgument(clang::Rewriter& rewriter,
                const clang::Token& token,
                const llvm::StringRef& mappedParameter) {
                const clang::SourceRange range(token.getLocation().getLocWithOffset(-1),
                    token.getLocation().getLocWithOffset(token.getLength() - 1));
                auto replacement = (llvm::Twine("\"") + mappedParameter + "\"").str();
                rewriter.ReplaceText(range, std::move(replacement));
            }

            /// Rewrites a macro argument use inside a macro when it is just a simple use
            /// and not stringified.
            void rewriteSimpleMacroArgument(clang::Rewriter& rewriter,
                const clang::Token& token,
                const llvm::StringRef& mappedParameter,
                unsigned hashCount) {
                const auto offset = -static_cast<int>(hashCount);
                const clang::SourceRange range(token.getLocation().getLocWithOffset(offset),
                    token.getLocation().getLocWithOffset(token.getLength() - 1));
                rewriter.ReplaceText(range, mappedParameter);
            }
        }  // namespace

        MacroSearch::MacroSearch(clang::CompilerInstance& compiler,
            Query& query)
            : _sourceManager(compiler.getSourceManager())
            , _languageOptions(compiler.getLangOpts())
            , _preprocessor(compiler.getPreprocessor())
            , _query(query) {
        }

        void MacroSearch::MacroExpands(const clang::Token& macroNameToken,
            const clang::MacroDefinition& macro,
            clang::SourceRange range,
            const clang::MacroArgs* arguments) {

            const auto* info = macro.getMacroInfo();
            const auto& loc = info->getDefinitionLoc();
            const auto macroname = _getSpelling(macroNameToken);
            auto defContext = _defCountMap.find(loc);
            if (defContext == _defCountMap.end())
                return; // This macro definition we don't care about
            ++defContext->second._count;

            if (_sourceManager.isInSystemHeader(loc) ||              //don't expand macros defined in a system header
                _sourceManager.isInSystemHeader(range.getBegin()) || //don't expand macros in headers that are in System headers
                !clang::Rewriter::isRewritable(range.getBegin())     //don't expand macros in headers that we cannot write to
                )
                return;
            auto original = getDefinitionText(*info, _sourceManager, _languageOptions);

            const auto mapping = _createParameterMap(*info, *arguments);
            if (info->isObjectLike() && !_query.options.wantsObjectExpand)
                return;
            if (info->isFunctionLike() && !_query.options.wantsFcnCallExpand)
                return;
            std::string text = _rewriteMacro(*info, mapping);

            Location location(loc, _sourceManager);

            if (info->isObjectLike()) {
                // - 1 because the range is inclusive
                const auto length = macroNameToken.getLength() - 1;
                range.setEnd(range.getBegin().getLocWithOffset(length));
            }
            _query._rewriter->ReplaceText(range, { text });
            Query::IndividualMacroInfo lmacro;
            lmacro.call.emplace(Range{ range, _sourceManager });
            lmacro.definition.emplace(std::move(location),
                std::move(original),
                std::move(text),
                /*isMacro=*/true);
            _query._macroInvocations.push_back(std::move(lmacro));

            //now update the defCountMap
            --defContext->second._count;
        }

        void MacroSearch::MacroDefined(const clang::Token & macroNameTok, const clang::MacroDirective * macroDirective)
        {
            const auto& loc = macroDirective->getMacroInfo()->getDefinitionLoc();
            //Return early if this is not a macro we can remove
            if (macroDirective->getMacroInfo()->isBuiltinMacro() || // For built-in macros example. __LINE__
                loc.isInvalid() ||                                  // For macros defined on the command line.
                _sourceManager.isInSystemHeader(loc)                // don't expand macros defined in a system header
                )
                return;
            if (macroNameTok.getKind() == clang::tok::identifier) {
                MacroContext ctx = { *macroDirective->getMacroInfo(), llvm::Optional<const clang::SourceRange>(), 0 };
                _defCountMap.insert({ loc, std::move(ctx) });
            }
        }

        void MacroSearch::MacroUndefined(const clang::Token &macroNameTok,
            const clang::MacroDefinition &macroDef)
        {
            const auto *defMacroInfo = macroDef.getMacroInfo();
            if (!defMacroInfo)
                return;
            const auto& loc = defMacroInfo->getDefinitionLoc();
            //Return early if this is not a macro we can remove
            if (loc.isInvalid() || defMacroInfo->isBuiltinMacro() || // For macros defined on the command line.
                _sourceManager.isInSystemHeader(loc)                 //don't expand macros defined in a system header
                )
                return;

            assert(defMacroInfo && loc.isValid() && "Could not find a #def for some #undef");
            auto defCtx = _defCountMap.find(loc);

            Routines::assertTrowIfFail(defCtx != _defCountMap.end(), (llvm::Twine("Could not find an #define for the #undef for ") + macroNameTok.getIdentifierInfo()->getName()).str());
            defCtx->second._undefRange.emplace( macroNameTok.getLocation(), macroNameTok.getEndLoc() );
        }

        void MacroSearch::EndOfMainFile()
        {
            if (!_query.options.wantsUnusedRemoved)
                return;
            for (const auto& ctxIt : _defCountMap)
            {               
                if (!clang::Rewriter::isRewritable(ctxIt.first)          //don't remove macros in headers we cannot write to
                    || ctxIt.second._defMacro.isUsedForHeaderGuard()     //don't remove macros from header guards
                    )
                    continue;
                Query::MacroDefCountMap::iterator macroDefIterator;
                if (!_sourceManager.isWrittenInMainFile(ctxIt.first)  ///\note:we're only focusing on removing macros from source files not header files
                    )
                {
                    const auto key = Location{ ctxIt.first,_sourceManager };
                    macroDefIterator = _query._macroDefinitionsInHeaders.find(key);
                    if (macroDefIterator == _query._macroDefinitionsInHeaders.end() ||
                        macroDefIterator->second.first < ctxIt.second._count) {
                        std::pair<size_t, llvm::Optional<Location>> val(ctxIt.second._count, llvm::Optional<Location>());
                        auto emplaceResult = _query._macroDefinitionsInHeaders.emplace(std::make_pair(key, val));
                        macroDefIterator = emplaceResult.first;
                    }
                    continue;
                } 
                if (ctxIt.second._count == 0)
                {
                    clang::Rewriter::RewriteOptions rwo;
                    rwo.RemoveLineIfEmpty = true;
                    auto decomposedMacroStart = _sourceManager.getDecomposedLoc(ctxIt.first);
                    bool Invalid = false;
                    auto hashLoc = _sourceManager.translateLineCol(decomposedMacroStart.first, _sourceManager.getLineNumber(decomposedMacroStart.first, decomposedMacroStart.second, &Invalid), 1);
                    clang::SourceRange macroRange = { hashLoc, ctxIt.second._defMacro.getDefinitionEndLoc() };
                    _query._rewriter->RemoveText(macroRange, rwo);
                    if (ctxIt.second._undefRange)
                    {
                        const auto& loc = ctxIt.second._undefRange->getBegin();
                        if (!clang::Rewriter::isRewritable(loc)                   //don't remove macros in headers we cannot write to
                            )
                            continue;
                        if (!_sourceManager.isWrittenInMainFile(loc)   //we're only focusing on removing macros from source files not header files
                            )
                        {
                            macroDefIterator->second.second = Location{ loc,_sourceManager };
                            continue;
                        }
                        decomposedMacroStart = _sourceManager.getDecomposedLoc(loc);
                        hashLoc = _sourceManager.translateLineCol(decomposedMacroStart.first, _sourceManager.getLineNumber(decomposedMacroStart.first, decomposedMacroStart.second, &Invalid), 1);
                        macroRange = { hashLoc, ctxIt.second._undefRange->getEnd() };
                        _query._rewriter->RemoveText(macroRange, rwo);
                    }
                }
            }
        }

        std::string MacroSearch::_rewriteMacro(const clang::MacroInfo& info,
            const ParameterMap& mapping) {
            clang::Rewriter rewriter(_sourceManager, _languageOptions);

            // Anytime we encounter a hash, we add 1 to this count. Once we are at an
            // identifier, we see how many hashes were right before it. If there are no
            // hashes, we just replace the identifier with the appropriate argument. If
            // there is one hash, we quote the argument. If there are two hashes (the
            // concatenation operator), we do the same thing as when there are no hashes,
            // since the concatenation is implicit for textual replacement. I.e. for
            // `foo_##arg_bar` where `arg` maps to `12` we can just replace this with
            // `foo_12_bar`.
            unsigned hashCount = 0;
            for (const auto& token : info.tokens()) {
                if (token.getKind() == clang::tok::identifier) {
                    const auto identifier = _getSpelling(token);

                    auto iterator = mapping.find(identifier);
                    if (iterator != mapping.end()) {
                        const auto mapped = iterator->getValue().str();

                        if (hashCount % 2 == 1) {
                            rewriteStringifiedMacroArgument(rewriter, token, mapped);
                        }
                        else {
                            rewriteSimpleMacroArgument(rewriter, token, mapped, hashCount);
                        }
                    }
                }

                if (token.getKind() == clang::tok::hash) {
                    hashCount += 1;
                }
                else if (token.getKind() == clang::tok::hashhash)
                {
                    hashCount += 2;
                }
                else {
                    hashCount = 0;
                }
            }

            const auto start = info.tokens_begin()->getLocation();
            const auto end = std::prev(info.tokens_end())->getEndLoc();
            return rewriter.getRewrittenText({ start, end });
        }

        MacroSearch::ParameterMap MacroSearch::_createParameterMap(
            const clang::MacroInfo& info, const clang::MacroArgs& arguments) {
            ParameterMap mapping;
            if (info.getNumArgs() == 0) return mapping;

            unsigned number = 0;
            for (const auto* parameter : info.args()) {
                const auto* firstToken = arguments.getUnexpArgument(number);
                auto numberOfTokens = arguments.getArgLength(firstToken);
                clang::TokenLexer lexer(firstToken,
                    numberOfTokens,
                    /*DisableExpansion=*/true,
                    false,
                    _preprocessor);

                llvm::SmallString<32> wholeArgument;
                while (numberOfTokens-- > 0) {
                    clang::Token token;
                    bool ok = lexer.Lex(token);
                    (void)ok;
                    Routines::assertTrowIfFail(ok, "Error lexing token in macro invocation");
                    wholeArgument += _getSpelling(token);
                }

                mapping[parameter->getName()] = std::move(wholeArgument);
                number += 1;
            }

            return mapping;
        }

        std::string MacroSearch::_getSpelling(const clang::Token& token) const {
            return clang::Lexer::getSpelling(token, _sourceManager, _languageOptions);
        }

    }  // namespace MacroExpand
}  // namespace tidy
