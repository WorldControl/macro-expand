#ifndef MACRO_EXPAND_PPTOOL_FACTORY_HPP
#define MACRO_EXPAND_PPTOOL_FACTORY_HPP

// Clang includes
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>

namespace tidy {
    struct Query;
}

namespace tidy {
    namespace MacroExpand {

        /// \ingroup MacroExpand
        ///
        /// Simple factory class to create a parameterized `MacroExpand` tool.
        ///
        /// This class is required because the standard `newFrontendAction` function
        /// does not allow passing parameters to an action.
        class ActionFactory : public clang::tooling::FrontendActionFactory {
        public:
            /// Constructor, taking the location the user invoked macro-expand with and
            /// the fresh `Query` object.
            explicit ActionFactory(Query& query);

            /// Creates the action of the symbol search phase.
            /// \returns A `MacroExpand::Action`.
            clang::FrontendAction* create() override;

        private:
            /// The newly created `Query` object.
            Query& _query;
        };
    }  // namespace MacroExpand
}  // namespace tidy

#endif  // MACRO_EXPAND_PPTOOL_FACTORY_HPP
