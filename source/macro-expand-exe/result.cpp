// Project includes
#include "misra-tidy/common/call-data.hpp"
#include "misra-tidy/common/definition-data.hpp"
#include "misra-tidy/common/routines.hpp"
#include "misra-tidy/macro-expand/options.hpp"
#include "result.hpp"

// Third party includes
#include <third-party/json.hpp>

// LLVM includes
#include <llvm/ADT/Optional.h>

// Standard includes
#include <cassert>
#include <utility>

namespace tidy {
    Result::Result(Query&& query)
        :_macros{ std::move(query._macroInvocations) }
        ,_needsJson(!query.options.wantsRewritten)
    {
    }

    nlohmann::json Result::toJson() const {
        nlohmann::json json;
        if (_needsJson){
            for (auto &macroInfo : _macros) {
                nlohmann::json macroJson;
                if (macroInfo.call.hasValue()) {
                    macroJson["call"] = macroInfo.call->extent.toJson();
                }

                if (macroInfo.definition.hasValue()) {
                    macroJson["definition"] = macroInfo.definition->toJson();
                }
                json.push_back(macroJson);
            }
        }
        return json.is_null() ? "" : json;
    }

}  // namespace tidy
