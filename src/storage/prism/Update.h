#ifndef STORM_STORAGE_PRISM_UPDATE_H_
#define STORM_STORAGE_PRISM_UPDATE_H_

#include <map>

#include "src/storage/prism/LocatedInformation.h"
#include "src/storage/prism/Assignment.h"

namespace storm {
    namespace prism {
        class Update : public LocatedInformation {
        public:
            /*!
             * Creates an update with the given expression specifying the likelihood and the mapping of variable to
             * their assignments.
             *
             * @param globalIndex The global index of the update.
             * @param likelihoodExpression An expression specifying the likelihood of this update.
             * @param assignments A map of variable names to their assignments.
             * @param filename The filename in which the variable is defined.
             * @param lineNumber The line number in which the variable is defined.
             */
            Update(uint_fast64_t index, storm::expressions::Expression const& likelihoodExpression, std::map<std::string, storm::prism::Assignment> const& booleanAssignments, std::map<std::string, storm::prism::Assignment> const& integerAssignments, std::string const& filename = "", uint_fast64_t lineNumber = 0);
            
            /*!
             * Creates a copy of the given update and performs the provided renaming.
             *
             * @param update The update that is to be copied.
             * @param newGlobalIndex The global index of the resulting update.
             * @param renaming A mapping from names that are to be renamed to the names they are to be replaced with.
             * @param filename The filename in which the variable is defined.
             * @param lineNumber The line number in which the variable is defined.
             */
            Update(Update const& update, uint_fast64_t newGlobalIndex, std::map<std::string, std::string> const& renaming, std::string const& filename = "", uint_fast64_t lineNumber = 0);
            
            // Create default implementations of constructors/assignment.
            Update() = default;
            Update(Update const& other) = default;
            Update& operator=(Update const& other)= default;
            Update(Update&& other) = default;
            Update& operator=(Update&& other) = default;
            
            /*!
             * Retrieves the expression for the likelihood of this update.
             *
             * @return The expression for the likelihood of this update.
             */
            storm::expressions::Expression const& getLikelihoodExpression() const;
            
            /*!
             * Retrieves the number of boolean assignments associated with this update.
             *
             * @return The number of boolean assignments associated with this update.
             */
            std::size_t getNumberOfBooleanAssignments() const;
            
            /*!
             * Retrieves the number of integer assignments associated with this update.
             *
             * @return The number of integer assignments associated with this update.
             */
            std::size_t getNumberOfIntegerAssignments() const;
            
            /*!
             * Retrieves a reference to the map of boolean variable names to their respective assignments.
             *
             * @return A reference to the map of boolean variable names to their respective assignments.
             */
            std::map<std::string, storm::prism::Assignment> const& getBooleanAssignments() const;
            
            /*!
             * Retrieves a reference to the map of integer variable names to their respective assignments.
             *
             * @return A reference to the map of integer variable names to their respective assignments.
             */
            std::map<std::string, storm::prism::Assignment> const& getIntegerAssignments() const;
            
            /*!
             * Retrieves a reference to the assignment for the boolean variable with the given name.
             *
             * @return A reference to the assignment for the boolean variable with the given name.
             */
            storm::prism::Assignment const& getBooleanAssignment(std::string const& variableName) const;
            
            /*!
             * Retrieves a reference to the assignment for the integer variable with the given name.
             *
             * @return A reference to the assignment for the integer variable with the given name.
             */
            storm::prism::Assignment const& getIntegerAssignment(std::string const& variableName) const;
            
            /*!
             * Retrieves the global index of the update, that is, a unique index over all modules.
             *
             * @return The global index of the update.
             */
            uint_fast64_t getGlobalIndex() const;
            
            friend std::ostream& operator<<(std::ostream& stream, Update const& assignment);
            
        private:
            // An expression specifying the likelihood of taking this update.
            storm::expressions::Expression likelihoodExpression;
            
            // A mapping of boolean variable names to their assignments in this update.
            std::map<std::string, storm::prism::Assignment> booleanAssignments;
            
            // A mapping of integer variable names to their assignments in this update.
            std::map<std::string, storm::prism::Assignment> integerAssignments;
            
            // The global index of the update.
            uint_fast64_t globalIndex;
        };
    } // namespace prism
} // namespace storm

#endif /* STORM_STORAGE_PRISM_UPDATE_H_ */