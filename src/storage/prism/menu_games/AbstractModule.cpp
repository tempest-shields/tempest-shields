#include "src/storage/prism/menu_games/AbstractModule.h"

#include "src/storage/prism/Module.h"

namespace storm {
    namespace prism {
        namespace menu_games {
            
            template <storm::dd::DdType DdType, typename ValueType>
            AbstractModule<DdType, ValueType>::AbstractModule(storm::expressions::ExpressionManager& expressionManager, storm::prism::Module const& module, std::vector<storm::expressions::Expression> const& initialPredicates, storm::utility::solver::SmtSolverFactory const& smtSolverFactory) : expressionManager(expressionManager), smtSolverFactory(smtSolverFactory), predicates(initialPredicates), commands(), module(module) {
                
                // For each concrete command, we create an abstract counterpart.
                for (auto const& command : module.getCommands()) {
                    commands.emplace_back(expressionManager, command, initialPredicates, smtSolverFactory);
                }
            }
            
            template class AbstractModule<storm::dd::DdType::CUDD, double>;
        }
    }
}