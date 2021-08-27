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

    private:
        storm::Environment _environment;
    };

    typedef ::testing::Types<
            DoubleViEnvironment
    > TestingTypes;

    TYPED_TEST_SUITE(ShieldGenerationSmgRpatlModelCheckerTest, TestingTypes,);

    TYPED_TEST(ShieldGenerationSmgRpatlModelCheckerTest, RightDecision) {
        typedef typename TestFixture::ValueType ValueType;

        // testing that no shield is created
        std::string formulasString = "<<hiker>> Pmax=? [ F <=3 \"target\" ]";

        // testing create shielding expressions
        formulasString += "; <preSafetyShieldLambda1, PreSafety, lambda=0.9> <<hiker>> Pmax=? [ F <=3 \"target\" ]";
        formulasString += "; <postSafetyShieldGamma1, PostSafety, gamma=0.9> <<hiker>> Pmax=? [ F <=3 \"target\" ]";
        formulasString += "; <preSafetyShieldLambda2, PreSafety, lambda=0.5> <<hiker>> Pmax=? [ F <=5 \"target\" ]";
        formulasString += "; <postSafetyShieldGamma2, PostSafety, gamma=0.5> <<hiker>> Pmax=? [ F <=5 \"target\" ]";
        formulasString += "; <preSafetyShieldLambda3, PreSafety, lambda=0> <<hiker, native>> Pmax=? [ F <=3 \"target\" ]";
        formulasString += "; <postSafetyShieldGamma3, PostSafety, gamma=0> <<hiker, native>> Pmax=? [ F <=3 \"target\" ]";
        formulasString += "; <preSafetyShieldLambda4, PreSafety, lambda=0.9> <<hiker>> Pmin=? [ F \"target\" ]";
        formulasString += "; <postSafetyShieldGamma4, PostSafety, gamma=0.9> <<hiker>> Pmin=? [ F \"target\" ]";

        auto modelFormulas = this->buildModelFormulas(STORM_TEST_RESOURCES_DIR "/smg/rightDecision.nm", formulasString);
        auto smg = std::move(modelFormulas.first);
        auto tasks = this->getTasks(modelFormulas.second);
        EXPECT_EQ(11ul, smg->getNumberOfStates());
        EXPECT_EQ(15ul, smg->getNumberOfTransitions());
        ASSERT_EQ(smg->getType(), storm::models::ModelType::Smg);
        EXPECT_EQ(14ull, smg->getNumberOfChoices());

        //std::unique_ptr<storm::modelchecker::CheckResult> result;
        storm::modelchecker::SparseSmgRpatlModelChecker<storm::models::sparse::Smg<ValueType>> checker(*smg);

        // first result must not be a shielding result
        auto result = checker.check(this->env(), tasks[0]);
        EXPECT_FALSE(      tasks[1].isShieldingTask());

        // shielding results
        storm::logic::ShieldingType type = storm::logic::ShieldingType::PreSafety;
        std::string filename = "preSafetyShieldLambda1";
        storm::logic::ShieldComparison comparison = storm::logic::ShieldComparison::Relative;
        double value = 0.9;
        auto preSafetyShieldingExpression = std::shared_ptr<storm::logic::ShieldExpression>(new storm::logic::ShieldExpression(type, filename, comparison, value));
        tasks[1].setShieldingExpression(preSafetyShieldingExpression);
        EXPECT_TRUE(tasks[1].isShieldingTask());

        result = checker.check(this->env(), tasks[1]);

        filename += ".shield";
        std::ifstream resultFile(filename);
        std::stringstream resultBuffer;
        resultBuffer << resultFile.rdbuf();
        std::string resultString = resultBuffer.str();
        std::cout << resultString << std::endl;
        std::remove(filename.c_str());
        // THIS WORKS!!

        //TODO: EXPECT_EQUAL: string from solution and string from comparison-file

        // TODO: build back the values, e.g. we do not need a method for computing string-shields
        //  - testfiles for comparison, make an folder for that and name it like the test case names
        //  - this also would work fine fith mdps, so finish this example, add it to mdp folder
        //  - then we can go to solve examples, we have to integrate a lot of examples...

        result = checker.check(this->env(), tasks[2]);
        result = checker.check(this->env(), tasks[3]);
        result = checker.check(this->env(), tasks[4]);
        result = checker.check(this->env(), tasks[5]);
        result = checker.check(this->env(), tasks[6]);
        result = checker.check(this->env(), tasks[7]);
        result = checker.check(this->env(), tasks[8]);
    }

    // TODO: create more test cases (files)
}
