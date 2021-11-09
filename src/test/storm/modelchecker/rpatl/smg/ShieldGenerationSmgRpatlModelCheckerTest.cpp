#include <io/file.h>
#include "test/storm_gtest.h"
#include "storm-config.h"

#include "storm/api/builder.h"
#include "storm-parsers/api/model_descriptions.h"
#include "storm/api/properties.h"
#include "storm-parsers/api/properties.h"

#include "storm/models/sparse/Smg.h"
#include "storm/modelchecker/rpatl/SparseSmgRpatlModelChecker.h"
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
    class ShieldGenerationSmgRpatlModelCheckerTest : public ::testing::Test {
    public:
        typedef typename TestType::ValueType ValueType;
        ShieldGenerationSmgRpatlModelCheckerTest() : _environment(TestType::createEnvironment()) {}
        storm::Environment const& env() const { return _environment; }

        std::pair<std::shared_ptr<storm::models::sparse::Smg<ValueType>>, std::vector<std::shared_ptr<storm::logic::Formula const>>> buildModelFormulas(std::string const& pathToPrismFile, std::string const& formulasAsString, std::string const& constantDefinitionString = "") const {
            std::pair<std::shared_ptr<storm::models::sparse::Smg<ValueType>>, std::vector<std::shared_ptr<storm::logic::Formula const>>> result;
            storm::prism::Program program = storm::api::parseProgram(pathToPrismFile);
            program = storm::utility::prism::preprocess(program, constantDefinitionString);
            result.second = storm::api::extractFormulasFromProperties(storm::api::parsePropertiesForPrismProgram(formulasAsString, program));
            result.first = storm::api::buildSparseModel<ValueType>(program, result.second)->template as<storm::models::sparse::Smg<ValueType>>();
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
            std::string compareFileName = STORM_TEST_RESOURCES_DIR "/shields/smg-shields/" + filename;
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

    TYPED_TEST_SUITE(ShieldGenerationSmgRpatlModelCheckerTest, TestingTypes,);

    TYPED_TEST(ShieldGenerationSmgRpatlModelCheckerTest, RightDecision) {
        typedef typename TestFixture::ValueType ValueType;

        // definition of shield file names
        std::vector<std::string> fileNames;
        fileNames.push_back("rightDecisionPreSafetyLambda09PmaxF3");
        fileNames.push_back("rightDecisionPreSafetyLambda09PminF3");
        fileNames.push_back("rightDecisionPreSafetyGamma09PmaxF3");
        fileNames.push_back("rightDecisionPreSafetyGamma09PminF3");
        fileNames.push_back("rightDecisionPostSafetyLambda09PmaxF3");
        fileNames.push_back("rightDecisionPostSafetyLambda09PminF3");
        fileNames.push_back("rightDecisionPostSafetyGamma09PmaxF3");
        fileNames.push_back("rightDecisionPostSafetyGamma09PminF3");
        fileNames.push_back("rightDecisionPreSafetyLambda05PmaxF5");
        fileNames.push_back("rightDecisionPreSafetyLambda05PminF5");
        fileNames.push_back("rightDecisionPreSafetyGamma05PmaxF5");
        fileNames.push_back("rightDecisionPreSafetyGamma05PminF5");
        fileNames.push_back("rightDecisionPostSafetyLambda05PmaxF5");
        fileNames.push_back("rightDecisionPostSafetyLambda05PminF5");
        fileNames.push_back("rightDecisionPostSafetyGamma05PmaxF5");
        fileNames.push_back("rightDecisionPostSafetyGamma05PminF5");

        // testing create shielding expressions
        std::string formulasString = "<" + fileNames[0] + ", PreSafety, lambda=0.9> <<hiker>> Pmax=? [ F <=3 \"target\" ]";
        formulasString += "; <" + fileNames[1] + ", PreSafety, lambda=0.9> <<native>> Pmin=? [ F <=3 \"target\" ]";
        formulasString += "; <" + fileNames[2] + ", PreSafety, gamma=0.9> <<hiker>> Pmax=? [ F <=3 \"target\" ]";
        formulasString += "; <" + fileNames[3] + ", PreSafety, gamma=0.9> <<native>> Pmin=? [ F <=3 \"target\" ]";
        formulasString += "; <" + fileNames[4] + ", PostSafety, lambda=0.9> <<hiker>> Pmax=? [ F <=3 \"target\" ]";
        formulasString += "; <" + fileNames[5] + ", PostSafety, lambda=0.9> <<native>> Pmin=? [ F <=3 \"target\" ]";
        formulasString += "; <" + fileNames[6] + ", PostSafety, gamma=0.9> <<hiker>> Pmax=? [ F <=3 \"target\" ]";
        formulasString += "; <" + fileNames[7] + ", PostSafety, gamma=0.9> <<native>> Pmin=? [ F <=3 \"target\" ]";

        formulasString += "; <" + fileNames[8] + ", PreSafety, lambda=0.5> <<hiker>> Pmax=? [ F <=5 \"target\" ]";
        formulasString += "; <" + fileNames[9] + ", PreSafety, lambda=0.5> <<native>> Pmin=? [ F <=5 \"target\" ]";
        formulasString += "; <" + fileNames[10] + ", PreSafety, gamma=0.5> <<hiker>> Pmax=? [ F <=5 \"target\" ]";
        formulasString += "; <" + fileNames[11] + ", PreSafety, gamma=0.5> <<native>> Pmin=? [ F <=5 \"target\" ]";
        formulasString += "; <" + fileNames[12] + ", PostSafety, lambda=0.5> <<hiker>> Pmax=? [ F <=5 \"target\" ]";
        formulasString += "; <" + fileNames[13] + ", PostSafety, lambda=0.5> <<native>> Pmin=? [ F <=5 \"target\" ]";
        formulasString += "; <" + fileNames[14] + ", PostSafety, gamma=0.5> <<hiker>> Pmax=? [ F <=5 \"target\" ]";
        formulasString += "; <" + fileNames[15] + ", PostSafety, gamma=0.5> <<native>> Pmin=? [ F <=5 \"target\" ]";

        auto modelFormulas = this->buildModelFormulas(STORM_TEST_RESOURCES_DIR "/smg/rightDecision.nm", formulasString);
        auto smg = std::move(modelFormulas.first);
        auto tasks = this->getTasks(modelFormulas.second);
        EXPECT_EQ(11ul, smg->getNumberOfStates());
        EXPECT_EQ(15ul, smg->getNumberOfTransitions());
        ASSERT_EQ(smg->getType(), storm::models::ModelType::Smg);
        EXPECT_EQ(14ull, smg->getNumberOfChoices());

        //std::unique_ptr<storm::modelchecker::CheckResult> result;
        storm::modelchecker::SparseSmgRpatlModelChecker<storm::models::sparse::Smg<ValueType>> checker(*smg);

        // definitions
        storm::logic::ShieldingType typePreSafety = storm::logic::ShieldingType::PreSafety;
        storm::logic::ShieldingType typePostSafety = storm::logic::ShieldingType::PostSafety;
        storm::logic::ShieldComparison comparisonRelative = storm::logic::ShieldComparison::Relative;
        storm::logic::ShieldComparison comparisonAbsolute = storm::logic::ShieldComparison::Absolute;
        double value09 = 0.9;
        double value05 = 0.5;
        std::string filename, shieldingString, compareFileString;

        // shielding results
        filename = fileNames[0];
        auto preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePreSafety, filename, comparisonRelative, value09));
        tasks[0].setShieldingExpression(preSafetyShieldingExpression);
        EXPECT_TRUE(tasks[0].isShieldingTask());
        auto result = checker.check(this->env(), tasks[0]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[1];
        preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePreSafety, filename, comparisonRelative, value09));
        tasks[1].setShieldingExpression(preSafetyShieldingExpression);
        EXPECT_TRUE(tasks[1].isShieldingTask());
        result = checker.check(this->env(), tasks[1]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[2];
        preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePreSafety, filename, comparisonAbsolute, value09));
        tasks[2].setShieldingExpression(preSafetyShieldingExpression);
        EXPECT_TRUE(tasks[2].isShieldingTask());
        result = checker.check(this->env(), tasks[2]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[3];
        preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePreSafety, filename, comparisonAbsolute, value09));
        tasks[3].setShieldingExpression(preSafetyShieldingExpression);
        EXPECT_TRUE(tasks[3].isShieldingTask());
        result = checker.check(this->env(), tasks[3]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[4];
        auto postSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePostSafety, filename, comparisonRelative, value09));
        tasks[4].setShieldingExpression(postSafetyShieldingExpression);
        EXPECT_TRUE(tasks[4].isShieldingTask());
        result = checker.check(this->env(), tasks[4]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[5];
        postSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePostSafety, filename, comparisonRelative, value09));
        tasks[5].setShieldingExpression(postSafetyShieldingExpression);
        EXPECT_TRUE(tasks[5].isShieldingTask());
        result = checker.check(this->env(), tasks[5]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[6];
        postSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePostSafety, filename, comparisonAbsolute, value09));
        tasks[6].setShieldingExpression(postSafetyShieldingExpression);
        EXPECT_TRUE(tasks[6].isShieldingTask());
        result = checker.check(this->env(), tasks[6]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[7];
        postSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePostSafety, filename, comparisonAbsolute, value09));
        tasks[7].setShieldingExpression(postSafetyShieldingExpression);
        EXPECT_TRUE(tasks[7].isShieldingTask());
        result = checker.check(this->env(), tasks[7]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);


        filename = fileNames[8];
        preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePreSafety, filename, comparisonRelative, value05));
        tasks[8].setShieldingExpression(preSafetyShieldingExpression);
        EXPECT_TRUE(tasks[8].isShieldingTask());
        result = checker.check(this->env(), tasks[8]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[9];
        preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePreSafety, filename, comparisonRelative, value05));
        tasks[9].setShieldingExpression(preSafetyShieldingExpression);
        EXPECT_TRUE(tasks[9].isShieldingTask());
        result = checker.check(this->env(), tasks[9]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[10];
        preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePreSafety, filename, comparisonAbsolute, value05));
        tasks[10].setShieldingExpression(preSafetyShieldingExpression);
        EXPECT_TRUE(tasks[10].isShieldingTask());
        result = checker.check(this->env(), tasks[10]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[11];
        preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePreSafety, filename, comparisonAbsolute, value05));
        tasks[11].setShieldingExpression(preSafetyShieldingExpression);
        EXPECT_TRUE(tasks[11].isShieldingTask());
        result = checker.check(this->env(), tasks[11]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[12];
        postSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePostSafety, filename, comparisonRelative, value05));
        tasks[12].setShieldingExpression(postSafetyShieldingExpression);
        EXPECT_TRUE(tasks[12].isShieldingTask());
        result = checker.check(this->env(), tasks[12]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[13];
        postSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePostSafety, filename, comparisonRelative, value05));
        tasks[13].setShieldingExpression(postSafetyShieldingExpression);
        EXPECT_TRUE(tasks[13].isShieldingTask());
        result = checker.check(this->env(), tasks[13]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[14];
        postSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePostSafety, filename, comparisonAbsolute, value05));
        tasks[14].setShieldingExpression(postSafetyShieldingExpression);
        EXPECT_TRUE(tasks[14].isShieldingTask());
        result = checker.check(this->env(), tasks[14]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);

        filename = fileNames[15];
        postSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(typePostSafety, filename, comparisonAbsolute, value05));
        tasks[15].setShieldingExpression(postSafetyShieldingExpression);
        EXPECT_TRUE(tasks[15].isShieldingTask());
        result = checker.check(this->env(), tasks[15]);
        this->getStringsToCompare(filename, shieldingString, compareFileString);
        EXPECT_EQ(shieldingString, compareFileString);
    }

    // TODO: create more test cases (files)
}
