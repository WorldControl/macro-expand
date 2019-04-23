
// Project includes
#include "misra-tidy/macro-expand/action-factory.hpp"
#include "misra-tidy/macro-expand/query.hpp"
#include "misra-tidy/macro-expand/action.hpp"

// Clang includes
#include <clang/Frontend/FrontendAction.h>

namespace tidy {
namespace MacroExpand {
ActionFactory::ActionFactory(Query& query)
: _query(query) {
}

clang::FrontendAction* ActionFactory::create() {
  return new MacroExpand::Action(_query);
}

}  // namespace MacroExpand
}  // namespace tidy
