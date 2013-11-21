/*
 * MarkovAutomaton.h
 *
 *  Created on: 07.11.2013
 *      Author: Christian Dehnert
 */

#ifndef STORM_MODELS_MA_H_
#define STORM_MODELS_MA_H_

#include "AbstractNondeterministicModel.h"
#include "AtomicPropositionsLabeling.h"
#include "src/storage/SparseMatrix.h"
#include "src/exceptions/InvalidArgumentException.h"
#include "src/settings/Settings.h"
#include "src/utility/vector.h"
#include "src/utility/matrix.h"

namespace storm {
	namespace models {

		template <class T>
		class MarkovAutomaton : public storm::models::AbstractNondeterministicModel<T> {

		public:
			MarkovAutomaton(storm::storage::SparseMatrix<T> const& transitionMatrix, storm::models::AtomicPropositionsLabeling const& stateLabeling,
							std::vector<uint_fast64_t>& nondeterministicChoiceIndices, storm::storage::BitVector const& markovianStates, std::vector<T> const& exitRates,
							boost::optional<std::vector<T>> const& optionalStateRewardVector, boost::optional<storm::storage::SparseMatrix<T>> const& optionalTransitionRewardMatrix,
							boost::optional<std::vector<storm::storage::VectorSet<uint_fast64_t>>> const& optionalChoiceLabeling)
							: AbstractNondeterministicModel<T>(transitionMatrix, stateLabeling, nondeterministicChoiceIndices, optionalStateRewardVector, optionalTransitionRewardMatrix, optionalChoiceLabeling),
                            markovianStates(markovianStates), exitRates(exitRates), closed(false) {
				if (this->hasTransitionRewards()) {
					if (!this->getTransitionRewardMatrix().isSubmatrixOf(this->getTransitionMatrix())) {
						LOG4CPLUS_ERROR(logger, "Transition reward matrix is not a submatrix of the transition matrix, i.e. there are rewards for transitions that do not exist.");
						throw storm::exceptions::InvalidArgumentException() << "There are transition rewards for nonexistent transitions.";
					}
				}
			}

			MarkovAutomaton(storm::storage::SparseMatrix<T>&& transitionMatrix,
							storm::models::AtomicPropositionsLabeling&& stateLabeling,
							std::vector<uint_fast64_t>&& nondeterministicChoiceIndices,
                            storm::storage::BitVector const& markovianStates, std::vector<T> const& exitRates,
							boost::optional<std::vector<T>>&& optionalStateRewardVector,
							boost::optional<storm::storage::SparseMatrix<T>>&& optionalTransitionRewardMatrix,
							boost::optional<std::vector<storm::storage::VectorSet<uint_fast64_t>>>&& optionalChoiceLabeling)
							: AbstractNondeterministicModel<T>(std::move(transitionMatrix), std::move(stateLabeling), std::move(nondeterministicChoiceIndices), std::move(optionalStateRewardVector), std::move(optionalTransitionRewardMatrix),
                                                               std::move(optionalChoiceLabeling)), markovianStates(markovianStates), exitRates(std::move(exitRates)), closed(false) {
		        if (this->hasTransitionRewards()) {
		            if (!this->getTransitionRewardMatrix().isSubmatrixOf(this->getTransitionMatrix())) {
		                LOG4CPLUS_ERROR(logger, "Transition reward matrix is not a submatrix of the transition matrix, i.e. there are rewards for transitions that do not exist.");
		                throw storm::exceptions::InvalidArgumentException() << "There are transition rewards for nonexistent transitions.";
		            }
		        }
			}

			MarkovAutomaton(MarkovAutomaton<T> const& markovAutomaton) : AbstractNondeterministicModel<T>(markovAutomaton), markovianStates(markovAutomaton.markovianStates), exitRates(markovAutomaton.exitRates), closed(markovAutomaton.closed) {
				// Intentionally left empty.
			}

			MarkovAutomaton(MarkovAutomaton<T>&& markovAutomaton) : AbstractNondeterministicModel<T>(std::move(markovAutomaton)), markovianStates(std::move(markovAutomaton.markovianStates)), exitRates(std::move(markovAutomaton.exitRates)), closed(markovAutomaton.closed) {
				// Intentionally left empty.
			}

			~MarkovAutomaton() {
				// Intentionally left empty.
			}

			storm::models::ModelType getType() const {
				return MA;
			}
            
            bool isClosed() const {
                return closed;
            }
            
            bool isHybridState(uint_fast64_t state) const {
                return isMarkovianState(state) && (this->getNondeterministicChoiceIndices()[state + 1] - this->getNondeterministicChoiceIndices()[state] > 1);
            }
                            
            bool isMarkovianState(uint_fast64_t state) const {
                return this->markovianStates.get(state);
            }
            
            bool isProbabilisticState(uint_fast64_t state) const {
                return !this->markovianStates.get(state);
            }
            
            std::vector<T> const& getExitRates() const {
                return this->exitRates;
            }
            
            storm::storage::BitVector const& getMarkovianStates() const {
                return this->markovianStates;
            }
            
            void close() {
                if (!closed) {
                    // First, count the number of hybrid states to know how many Markovian choices
                    // will be removed.
                    uint_fast64_t numberOfHybridStates = 0;
                    for (uint_fast64_t state = 0; state < this->getNumberOfStates(); ++state) {
                        if (this->isHybridState(state)) {
                            ++numberOfHybridStates;
                        }
                    }
                    
                    // Then compute how many rows the new matrix is going to have.
                    uint_fast64_t newNumberOfRows = this->getNumberOfChoices() - numberOfHybridStates;
                    
                    // Create the matrix for the new transition relation and the corresponding nondeterministic choice vector.
                    storm::storage::SparseMatrix<T> newTransitionMatrix(newNumberOfRows, this->getNumberOfStates());
                    newTransitionMatrix.initialize();
                    std::vector<uint_fast64_t> newNondeterministicChoiceIndices(this->getNumberOfStates() + 1);
                    
                    // Now copy over all choices that need to be kept.
                    uint_fast64_t currentChoice = 0;
                    for (uint_fast64_t state = 0; state < this->getNumberOfStates(); ++state) {
                        // If the state is a hybrid state, closing it will make it a probabilistic state, so we remove the Markovian marking.
                        if (this->isHybridState(state)) {
                            this->markovianStates.set(state, false);
                        }
                        
                        // Record the new beginning of choices of this state.
                        newNondeterministicChoiceIndices[state] = currentChoice;
                        
                        typename storm::storage::SparseMatrix<T>::ConstRowIterator rowIt = this->transitionMatrix.begin(this->nondeterministicChoiceIndices[state]), rowIte = this->transitionMatrix.end(this->nondeterministicChoiceIndices[state + 1] - 1);
                        
                        // If we are currently treating a hybrid state, we need to skip its first choice.
                        if (this->isHybridState(state)) {
                            ++rowIt;
                            
                            // Remove the Markovian state marking.
                            this->markovianStates.set(state, false);
                        }

                        for (; rowIt != rowIte; ++rowIt) {
                            for (typename storm::storage::SparseMatrix<T>::ConstIterator succIt = rowIt.begin(), succIte = rowIt.end(); succIt != succIte; ++succIt) {
                                newTransitionMatrix.insertNextValue(currentChoice, succIt.column(), succIt.value());
                            }
                            ++currentChoice;
                        }
                    }
                    
                    // Put a sentinel element at the end.
                    newNondeterministicChoiceIndices.back() = currentChoice;
                    
                    // Finalize the matrix and put the new transition data in place.
                    newTransitionMatrix.finalize();
                    this->transitionMatrix = std::move(newTransitionMatrix);
                    this->nondeterministicChoiceIndices = std::move(newNondeterministicChoiceIndices);
                    
                    // Mark the automaton as closed.
                    closed = true;
                }
            }
            
            virtual std::shared_ptr<AbstractModel<T>> applyScheduler(storm::storage::Scheduler const& scheduler) const override {
                if (!closed) {
                    throw storm::exceptions::InvalidStateException() << "Applying a scheduler to a non-closed Markov automaton is illegal; it needs to be closed first.";
                }
                
                storm::storage::SparseMatrix<T> newTransitionMatrix = storm::utility::matrix::applyScheduler(this->getTransitionMatrix(), this->getNondeterministicChoiceIndices(), scheduler);
                                
                // Construct the new nondeterministic choice indices for the resulting matrix.
                std::vector<uint_fast64_t> nondeterministicChoiceIndices(this->getNumberOfStates() + 1);
                for (uint_fast64_t state = 0; state < this->getNumberOfStates(); ++state) {
                    nondeterministicChoiceIndices[state] = state;
                }
                nondeterministicChoiceIndices[this->getNumberOfStates()] = this->getNumberOfStates();
            
                return std::shared_ptr<AbstractModel<T>>(new MarkovAutomaton(newTransitionMatrix, this->getStateLabeling(), nondeterministicChoiceIndices, markovianStates, exitRates, this->hasStateRewards() ? this->getStateRewardVector() : boost::optional<std::vector<T>>(), this->hasTransitionRewards() ? this->getTransitionRewardMatrix() :  boost::optional<storm::storage::SparseMatrix<T>>(), this->hasChoiceLabeling() ? this->getChoiceLabeling() : boost::optional<std::vector<storm::storage::VectorSet<uint_fast64_t>>>()));
            }
        
        virtual void writeDotToStream(std::ostream& outStream, bool includeLabeling = true, storm::storage::BitVector const* subsystem = nullptr, std::vector<T> const* firstValue = nullptr, std::vector<T> const* secondValue = nullptr, std::vector<uint_fast64_t> const* stateColoring = nullptr, std::vector<std::string> const* colors = nullptr, std::vector<uint_fast64_t>* scheduler = nullptr, bool finalizeOutput = true) const override {
        AbstractModel<T>::writeDotToStream(outStream, includeLabeling, subsystem, firstValue, secondValue, stateColoring, colors, scheduler, false);
        
        // Write the probability distributions for all the states.
        auto rowIt = this->transitionMatrix.begin();
        for (uint_fast64_t state = 0, highestStateIndex = this->getNumberOfStates() - 1; state <= highestStateIndex; ++state) {
            uint_fast64_t rowCount = this->getNondeterministicChoiceIndices()[state + 1] - this->getNondeterministicChoiceIndices()[state];
            bool highlightChoice = true;
            
            // For this, we need to iterate over all available nondeterministic choices in the current state.
            for (uint_fast64_t row = 0; row < rowCount; ++row, ++rowIt) {
                if (scheduler != nullptr) {
                    // If the scheduler picked the current choice, we will not make it dotted, but highlight it.
                    if ((*scheduler)[state] == row) {
                        highlightChoice = true;
                    } else {
                        highlightChoice = false;
                    }
                }
                
                // If it's not a Markovian state or the current row is the first choice for this state, then we
                // are dealing with a probabilitic choice.
                if (!markovianStates.get(state) || row != 0) {
                    // For each nondeterministic choice, we draw an arrow to an intermediate node to better display
                    // the grouping of transitions.
                    outStream << "\t\"" << state << "c" << row << "\" [shape = \"point\"";
                    
                    // If we were given a scheduler to highlight, we do so now.
                    if (scheduler != nullptr) {
                        if (highlightChoice) {
                            outStream << ", fillcolor=\"red\"";
                        }
                    }
                    outStream << "];" << std::endl;
                    
                    outStream << "\t" << state << " -> \"" << state << "c" << row << "\"";
                    
                    // If we were given a scheduler to highlight, we do so now.
                    if (scheduler != nullptr) {
                        if (highlightChoice) {
                            outStream << " [color=\"red\", penwidth = 2]";
                        } else {
                            outStream << " [style = \"dotted\"]";
                        }
                    }
                    outStream << ";" << std::endl;
                    
                    // Now draw all probabilitic arcs that belong to this nondeterminstic choice.
                    for (auto transitionIt = rowIt.begin(), transitionIte = rowIt.end(); transitionIt != transitionIte; ++transitionIt) {
                        if (subsystem == nullptr || subsystem->get(transitionIt.column())) {
                            outStream << "\t\"" << state << "c" << row << "\" -> " << transitionIt.column() << " [ label= \"" << transitionIt.value() << "\" ]";
                            
                            // If we were given a scheduler to highlight, we do so now.
                            if (scheduler != nullptr) {
                                if (highlightChoice) {
                                    outStream << " [color=\"red\", penwidth = 2]";
                                } else {
                                    outStream << " [style = \"dotted\"]";
                                }
                            }
                            outStream << ";" << std::endl;
                        }
                    }
                    } else {
                        // In this case we are emitting a Markovian choice, so draw the arrows directly to the target states.
                        for (auto transitionIt = rowIt.begin(), transitionIte = rowIt.end(); transitionIt != transitionIte; ++transitionIt) {
                            if (subsystem == nullptr || subsystem->get(transitionIt.column())) {
                                outStream << "\t\"" << state << "\" -> " << transitionIt.column() << " [ label= \"" << transitionIt.value() << "\" ]";
                            }
                        }
                    }
                    }
                    }
                    
                    if (finalizeOutput) {
                        outStream << "}" << std::endl;
                    }
                    }
		private:

            storm::storage::BitVector markovianStates;
			std::vector<T> exitRates;
            bool closed;

		};
	}
}

#endif /* STORM_MODELS_MA_H_ */