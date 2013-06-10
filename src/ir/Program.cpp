/*
 * Program.cpp
 *
 *  Created on: 12.01.2013
 *      Author: Christian Dehnert
 */

#include <sstream>
#include <iostream>

#include "Program.h"
#include "exceptions/InvalidArgumentException.h"
#include "src/exceptions/OutOfRangeException.h"

#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"
extern log4cplus::Logger logger;

namespace storm {
    namespace ir {
        
        Program::Program() : modelType(UNDEFINED), booleanUndefinedConstantExpressions(), integerUndefinedConstantExpressions(), doubleUndefinedConstantExpressions(), modules(), rewards(), actions(), actionsToModuleIndexMap() {
            // Nothing to do here.
        }
        
        Program::Program(ModelType modelType, std::map<std::string, std::shared_ptr<storm::ir::expressions::BooleanConstantExpression>> booleanUndefinedConstantExpressions, std::map<std::string, std::shared_ptr<storm::ir::expressions::IntegerConstantExpression>> integerUndefinedConstantExpressions, std::map<std::string, std::shared_ptr<storm::ir::expressions::DoubleConstantExpression>> doubleUndefinedConstantExpressions, std::vector<storm::ir::Module> modules, std::map<std::string, storm::ir::RewardModel> rewards, std::map<std::string, std::shared_ptr<storm::ir::expressions::BaseExpression>> labels)
        : modelType(modelType), booleanUndefinedConstantExpressions(booleanUndefinedConstantExpressions), integerUndefinedConstantExpressions(integerUndefinedConstantExpressions), doubleUndefinedConstantExpressions(doubleUndefinedConstantExpressions), modules(modules), rewards(rewards), labels(labels), actionsToModuleIndexMap() {
            // Now build the mapping from action names to module indices so that the lookup can later be performed quickly.
            for (unsigned int moduleId = 0; moduleId < this->modules.size(); moduleId++) {
                for (auto const& action : this->modules[moduleId].getActions()) {
                    if (this->actionsToModuleIndexMap.count(action) == 0) {
                        this->actionsToModuleIndexMap[action] = std::set<uint_fast64_t>();
                    }
                    this->actionsToModuleIndexMap[action].insert(moduleId);
                    this->actions.insert(action);
                }
            }
        }
        
        Program::ModelType Program::getModelType() const {
            return modelType;
        }
        
        std::string Program::toString() const {
            std::stringstream result;
            switch (modelType) {
                case UNDEFINED: result << "undefined"; break;
                case DTMC: result << "dtmc"; break;
                case CTMC: result << "ctmc"; break;
                case MDP: result << "mdp"; break;
                case CTMDP: result << "ctmdp"; break;
            }
            result << std::endl;
            
            for (auto const& element : booleanUndefinedConstantExpressions) {
                result << "const bool " << element.first << " [" << element.second->toString() << "]" << ";" << std::endl;
            }
            for (auto const& element : integerUndefinedConstantExpressions) {
                result << "const int " << element.first << " [" << element.second->toString() << "]" << ";" << std::endl;
            }
            for (auto const& element : doubleUndefinedConstantExpressions) {
                result << "const double " << element.first << " [" << element.second->toString() << "]" << ";" << std::endl;
            }
            result << std::endl;
            
            for (auto const& module : modules) {
                result << module.toString() << std::endl;
            }
            
            for (auto const& rewardModel : rewards) {
                result << rewardModel.first << ": " << rewardModel.second.toString() << std::endl;
            }
            
            for (auto const& label : labels) {
                result << "label " << label.first << " = " << label.second->toString() <<";" << std::endl;
            }
            
            return result.str();
        }
        
        uint_fast64_t Program::getNumberOfModules() const {
            return this->modules.size();
        }
        
        storm::ir::Module const& Program::getModule(uint_fast64_t index) const {
            return this->modules[index];
        }
        
        std::set<std::string> const& Program::getActions() const {
            return this->actions;
        }
        
        std::set<uint_fast64_t> const& Program::getModulesByAction(std::string const& action) const {
            auto actionModuleSetPair = this->actionsToModuleIndexMap.find(action);
            if (actionModuleSetPair == this->actionsToModuleIndexMap.end()) {
                LOG4CPLUS_ERROR(logger, "Action name '" << action << "' does not exist.");
                throw storm::exceptions::OutOfRangeException() << "Action name '" << action << "' does not exist.";
            }
            return actionModuleSetPair->second;
        }
        
        storm::ir::RewardModel const& Program::getRewardModel(std::string const& name) const {
            auto nameRewardModelPair = this->rewards.find(name);
            if (nameRewardModelPair == this->rewards.end()) {
                LOG4CPLUS_ERROR(logger, "Reward model '" << name << "' does not exist.");
                throw storm::exceptions::OutOfRangeException() << "Reward model '" << name << "' does not exist.";
            }
            return nameRewardModelPair->second;
        }
        
        std::map<std::string, std::shared_ptr<storm::ir::expressions::BaseExpression>> const& Program::getLabels() const {
            return this->labels;
        }
        
    } // namespace ir
} // namepsace storm