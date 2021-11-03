#include "ShieldHandling.h"

namespace tempest {
    namespace shields {
        std::string shieldFilename(std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression) {
            return shieldingExpression->getFilename() + ".shield";
        }

        template<typename ValueType, typename IndexType>
        void createShield(std::shared_ptr<storm::models::sparse::Model<ValueType>> model, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates) {
            std::ofstream stream;
            storm::utility::openFile(shieldFilename(shieldingExpression), stream);
            if(coalitionStates.is_initialized()) coalitionStates.get().complement();
            if(shieldingExpression->isPreSafetyShield()) {
                PreShield<ValueType, IndexType> shield(model->getTransitionMatrix().getRowGroupIndices(), choiceValues, shieldingExpression, optimizationDirection, relevantStates, coalitionStates);
                shield.construct().printToStream(stream, shieldingExpression, model);
            } else if(shieldingExpression->isPostSafetyShield()) {
                PostShield<ValueType, IndexType> shield(model->getTransitionMatrix().getRowGroupIndices(), choiceValues, shieldingExpression, optimizationDirection, relevantStates, coalitionStates);
                shield.construct().printToStream(stream, shieldingExpression, model);
            } else {
                STORM_LOG_THROW(false, storm::exceptions::InvalidArgumentException, "Unknown Shielding Type: " + shieldingExpression->typeToString());
                storm::utility::closeFile(stream);
            }
            storm::utility::closeFile(stream);
        }

        template<typename ValueType, typename IndexType>
        void createQuantitativeShield(std::shared_ptr<storm::models::sparse::Model<ValueType>> model, std::vector<ValueType> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates) {
            std::ofstream stream;
            storm::utility::openFile(shieldFilename(shieldingExpression), stream);
            if(coalitionStates.is_initialized()) coalitionStates.get().complement(); // TODO CHECK THIS!!!
            if(shieldingExpression->isOptimalPreShield()) {
                PreShield<ValueType, IndexType> shield(model->getTransitionMatrix().getRowGroupIndices(), choiceValues, shieldingExpression, optimizationDirection, relevantStates, coalitionStates);
                shield.construct().printToStream(stream, shieldingExpression, model);
            } else if(shieldingExpression->isOptimalPostShield()) {
                PostShield<ValueType, IndexType> shield(model->getTransitionMatrix().getRowGroupIndices(), choiceValues, shieldingExpression, optimizationDirection, relevantStates, coalitionStates);
                shield.construct().printToStream(stream, shieldingExpression, model);
            } else {
                STORM_LOG_THROW(false, storm::exceptions::InvalidArgumentException, "Unknown Shielding Type: " + shieldingExpression->typeToString());
                storm::utility::closeFile(stream);
            }
            storm::utility::closeFile(stream);
        }
        // Explicitly instantiate appropriate
        template void createShield<double, typename storm::storage::SparseMatrix<double>::index_type>(std::shared_ptr<storm::models::sparse::Model<double>> model, std::vector<double> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates);
        template void createQuantitativeShield<double, typename storm::storage::SparseMatrix<double>::index_type>(std::shared_ptr<storm::models::sparse::Model<double>> model, std::vector<double> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates);
#ifdef STORM_HAVE_CARL
        template void createShield<storm::RationalNumber, typename storm::storage::SparseMatrix<storm::RationalNumber>::index_type>(std::shared_ptr<storm::models::sparse::Model<storm::RationalNumber>> model, std::vector<storm::RationalNumber> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates);
        template void createQuantitativeShield<storm::RationalNumber, typename storm::storage::SparseMatrix<storm::RationalNumber>::index_type>(std::shared_ptr<storm::models::sparse::Model<storm::RationalNumber>> model, std::vector<storm::RationalNumber> const& choiceValues, std::shared_ptr<storm::logic::ShieldExpression const> const& shieldingExpression, storm::OptimizationDirection optimizationDirection, storm::storage::BitVector relevantStates, boost::optional<storm::storage::BitVector> coalitionStates);
#endif
    }
}
