#include <io/file.h>
#include "test/storm_gtest.h"
#include "storm-config.h"

#include "storm/api/builder.h"
#include "storm-parsers/api/model_descriptions.h"
#include "storm/api/properties.h"
#include "storm-parsers/api/properties.h"

#include "storm/models/sparse/Smg.h"
#include "storm/modelchecker/prctl/SparseMdpPrctlModelChecker.h"
#include "storm/modelchecker/results/QuantitativeCheckResult.h"
#include "storm/modelchecker/results/QualitativeCheckResult.h"
#include "storm/environment/solver/MinMaxSolverEnvironment.h"
#include "storm/environment/solver/TopologicalSolverEnvironment.h"
#include "storm/environment/solver/MultiplierEnvironment.h"
#include "storm/settings/modules/CoreSettings.h"
#include "storm/logic/Formulas.h"
#include "storm/exceptions/UncheckedRequirementException.h"

namespace {
    class DoubleViEnvironment {
    public:
        typedef double ValueType;
        static storm::Environment createEnvironment() {
            storm::Environment env;
            env.solver().minMax().setMethod(storm::solver::MinMaxMethod::ValueIteration);
            env.solver().minMax().setPrecision(storm::utility::convertNumber<storm::RationalNumber>(1e-8));
            return env;
        }
    };

    template<typename TestType>
    class ShieldGenerationMdpPrctlModelCheckerTest : public ::testing::Test {
    public:
        typedef typename TestType::ValueType ValueType;
        ShieldGenerationMdpPrctlModelCheckerTest() : _environment(TestType::createEnvironment()) {}
        storm::Environment const& env() const { return _environment; }

        std::pair<std::shared_ptr<storm::models::sparse::Mdp<ValueType>>, std::vector<std::shared_ptr<storm::logic::Formula const>>> buildModelFormulas(std::string const& pathToPrismFile, std::string const& formulasAsString, std::string const& constantDefinitionString = "") const {
            std::pair<std::shared_ptr<storm::models::sparse::Mdp<ValueType>>, std::vector<std::shared_ptr<storm::logic::Formula const>>> result;
            storm::prism::Program program = storm::api::parseProgram(pathToPrismFile);
            program = storm::utility::prism::preprocess(program, constantDefinitionString);
            result.second = storm::api::extractFormulasFromProperties(storm::api::parsePropertiesForPrismProgram(formulasAsString, program));
            result.first = storm::api::buildSparseModel<ValueType>(program, result.second)->template as<storm::models::sparse::Mdp<ValueType>>();
            return result;
        }

        std::vector<storm::modelchecker::CheckTask<storm::logic::Formula, ValueType>> getTasks(std::vector<std::shared_ptr<storm::logic::Formula const>> const& formulas) const {
            std::vector<storm::modelchecker::CheckTask<storm::logic::Formula, ValueType>> result;
            for (auto const& f : formulas) {
                result.emplace_back(*f);
            }
            return result;
        }

        ValueType parseNumber(std::string const& input) const { return storm::utility::convertNumber<ValueType>(input);}

        void convertShieldingFileToString(std::string filename, std::string &shieldingString) {
            filename += shieldFiletype;
            std::ifstream resultFile(filename);
            std::stringstream resultBuffer;
            resultBuffer << resultFile.rdbuf();
            shieldingString = resultBuffer.str();
        }

        void getStringsToCompare(std::string filename, std::string &shieldingString, std::string &compareFileString) {
            this->convertShieldingFileToString(filename, shieldingString);
            std::string compareFileName = STORM_TEST_RESOURCES_DIR "/shields/mdp-shields/" + filename;
            this->convertShieldingFileToString(compareFileName, compareFileString);
            filename += shieldFiletype;
            std::remove(filename.c_str());
        }

    private:
        storm::Environment _environment;
        std::string shieldFiletype = ".shield";
    };

    typedef ::testing::Types<
    DoubleViEnvironment
    > TestingTypes;

    TYPED_TEST_SUITE(ShieldGenerationMdpPrctlModelCheckerTest, TestingTypes,);

    TYPED_TEST(ShieldGenerationMdpPrctlModelCheckerTest, DieSelection) {
    typedef typename TestFixture::ValueType ValueType;

    // definition of shield file names
    std::vector<std::string> fileNames;
    fileNames.push_back("dieSelectionPreSafetylambda08Pmax");
    fileNames.push_back("dieSelectionPreSafetygamma08Pmax");
    fileNames.push_back("dieSelectionPreSafetylambda08Pmin");
    fileNames.push_back("dieSelectionPreSafetygamma08Pmin");

    fileNames.push_back("dieSelectionPostSafetylambda07Pmax");
    fileNames.push_back("dieSelectionPostSafetygamma07Pmax");
    fileNames.push_back("dieSelectionPostSafetylambda07Pmin");
    fileNames.push_back("dieSelectionPostSafetygamma07Pmin");

    // testing create shielding expressions
    std::string formulasString = "<" + fileNames[0] + ", PreSafety, lambda=0.8> Pmax=? [ F <5 \"done\" ]";
    formulasString += "; <" + fileNames[1] + ", PreSafety, gamma=0.8> Pmax=? [ F <5 \"done\" ]";
    formulasString += "; <" + fileNames[2] + ", PreSafety, lambda=0.8> Pmin=? [ F <5 \"done\" ]";
    formulasString += "; <" + fileNames[3] + ", PreSafety, gamma=0.8> Pmin=? [ F <5 \"done\" ]";

    formulasString += "; <" + fileNames[4] + ", PostSafety, lambda=0.7> Pmax=? [ F <6 \"two\" ]";
    formulasString += "; <" + fileNames[5] + ", PostSafety, gamma=0.7> Pmax=? [ F <6 \"two\" ]";
    formulasString += "; <" + fileNames[6] + ", PostSafety, lambda=0.7> Pmin=? [ F <6 \"two\" ]";
    formulasString += "; <" + fileNames[7] + ", PostSafety, gamma=0.7> Pmin=? [ F <6 \"two\" ]";

    auto modelFormulas = this->buildModelFormulas(STORM_TEST_RESOURCES_DIR "/mdp/die_selection.nm", formulasString);
    auto mdp = std::move(modelFormulas.first);
    auto tasks = this->getTasks(modelFormulas.second);
    EXPECT_EQ(13ul, mdp->getNumberOfStates());
    EXPECT_EQ(48ul, mdp->getNumberOfTransitions());
    ASSERT_EQ(mdp->getType(), storm::models::ModelType::Mdp);
    EXPECT_EQ(27ul, mdp->getNumberOfChoices());

    storm::modelchecker::SparseMdpPrctlModelChecker<storm::models::sparse::Mdp<ValueType>> checker(*mdp);

    // definitions
    storm::logic::ShieldingType typePreSafety = storm::logic::ShieldingType::PreSafety;
    storm::logic::ShieldingType typePostSafety = storm::logic::ShieldingType::PostSafety;
    storm::logic::ShieldComparison comparisonRelative = storm::logic::ShieldComparison::Relative;
    storm::logic::ShieldComparison comparisonAbsolute = storm::logic::ShieldComparison::Absolute;
    double value08 = 0.8;
    double value07 = 0.7;
    std::string filename, shieldingString, compareFileString;

    // shielding results
    filename = fileNames[0];
    auto preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePreSafety, filename, comparisonRelative, value08));
    tasks[0].setShieldingExpression(preSafetyShieldingExpression);
    EXPECT_TRUE(tasks[0].isShieldingTask());
    auto result = checker.check(this->env(), tasks[0]);
    this->getStringsToCompare(filename, shieldingString, compareFileString);
    EXPECT_EQ(shieldingString, compareFileString);

    filename = fileNames[1];
    preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePreSafety, filename, comparisonAbsolute, value08));
    tasks[1].setShieldingExpression(preSafetyShieldingExpression);
    EXPECT_TRUE(tasks[1].isShieldingTask());
    result = checker.check(this->env(), tasks[1]);
    this->getStringsToCompare(filename, shieldingString, compareFileString);
    EXPECT_EQ(shieldingString, compareFileString);

    filename = fileNames[2];
    preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePreSafety, filename, comparisonRelative, value08));
    tasks[2].setShieldingExpression(preSafetyShieldingExpression);
    EXPECT_TRUE(tasks[2].isShieldingTask());
    result = checker.check(this->env(), tasks[2]);
    this->getStringsToCompare(filename, shieldingString, compareFileString);
    EXPECT_EQ(shieldingString, compareFileString);

    filename = fileNames[3];
    preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePreSafety, filename, comparisonAbsolute, value08));
    tasks[3].setShieldingExpression(preSafetyShieldingExpression);
    EXPECT_TRUE(tasks[3].isShieldingTask());
    result = checker.check(this->env(), tasks[3]);
    this->getStringsToCompare(filename, shieldingString, compareFileString);
    EXPECT_EQ(shieldingString, compareFileString);

    filename = fileNames[4];
    auto postSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePostSafety, filename, comparisonRelative, value07));
    tasks[4].setShieldingExpression(postSafetyShieldingExpression);
    EXPECT_TRUE(tasks[4].isShieldingTask());
    result = checker.check(this->env(), tasks[4]);
    this->getStringsToCompare(filename, shieldingString, compareFileString);
    EXPECT_EQ(shieldingString, compareFileString);

    filename = fileNames[5];
    postSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePostSafety, filename, comparisonAbsolute, value07));
    tasks[5].setShieldingExpression(postSafetyShieldingExpression);
    EXPECT_TRUE(tasks[5].isShieldingTask());
    result = checker.check(this->env(), tasks[5]);
    this->getStringsToCompare(filename, shieldingString, compareFileString);
    EXPECT_EQ(shieldingString, compareFileString);

    filename = fileNames[6];
    postSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePostSafety, filename, comparisonRelative, value07));
    tasks[6].setShieldingExpression(postSafetyShieldingExpression);
    EXPECT_TRUE(tasks[6].isShieldingTask());
    result = checker.check(this->env(), tasks[6]);
    this->getStringsToCompare(filename, shieldingString, compareFileString);
    EXPECT_EQ(shieldingString, compareFileString);

    filename = fileNames[7];
    postSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePostSafety, filename, comparisonAbsolute, value07));
    tasks[7].setShieldingExpression(postSafetyShieldingExpression);
    EXPECT_TRUE(tasks[7].isShieldingTask());
    result = checker.check(this->env(), tasks[7]);
    this->getStringsToCompare(filename, shieldingString, compareFileString);
    EXPECT_EQ(shieldingString, compareFileString);
}

// TODO: create more test cases (files)
}
