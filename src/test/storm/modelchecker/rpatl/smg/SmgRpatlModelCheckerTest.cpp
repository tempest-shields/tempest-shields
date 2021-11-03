#include "test/storm_gtest.h"
#include "storm-config.h"

#include "storm/api/builder.h"
#include "storm-parsers/api/model_descriptions.h"
#include "storm/api/properties.h"
#include "storm-parsers/api/properties.h"

#include "storm/models/sparse/Smg.h"
#include "storm/modelchecker/rpatl/SparseSmgRpatlModelChecker.h"
#include "storm/modelchecker/results/QuantitativeCheckResult.h"
#include "storm/modelchecker/results/ExplicitQualitativeCheckResult.h"
#include "storm/modelchecker/results/QualitativeCheckResult.h"
#include "storm/environment/solver/MinMaxSolverEnvironment.h"
#include "storm/environment/solver/TopologicalSolverEnvironment.h"
#include "storm/environment/solver/MultiplierEnvironment.h"
#include "storm/settings/modules/CoreSettings.h"
#include "storm/logic/Formulas.h"
#include "storm/exceptions/UncheckedRequirementException.h"

namespace {

    enum class SmgEngine {PrismSparse};

    class SparseDoubleValueIterationGmmxxGaussSeidelMultEnvironment {
    public:
        static const SmgEngine engine = SmgEngine::PrismSparse;
        static const bool isExact = false;
        typedef double ValueType;
        typedef storm::models::sparse::Smg<ValueType> ModelType;
        static storm::Environment createEnvironment() {
            storm::Environment env;
            env.solver().minMax().setMethod(storm::solver::MinMaxMethod::ValueIteration);
            env.solver().minMax().setPrecision(storm::utility::convertNumber<storm::RationalNumber>(1e-10));
            env.solver().minMax().setMultiplicationStyle(storm::solver::MultiplicationStyle::GaussSeidel);
            env.solver().multiplier().setType(storm::solver::MultiplierType::Gmmxx);
            return env;
        }
    };

    class SparseDoubleValueIterationGmmxxRegularMultEnvironment {
    public:
        static const SmgEngine engine = SmgEngine::PrismSparse;
        static const bool isExact = false;
        typedef double ValueType;
        typedef storm::models::sparse::Smg<ValueType> ModelType;
        static storm::Environment createEnvironment() {
            storm::Environment env;
            env.solver().minMax().setMethod(storm::solver::MinMaxMethod::ValueIteration);
            env.solver().minMax().setPrecision(storm::utility::convertNumber<storm::RationalNumber>(1e-10));
            env.solver().minMax().setMultiplicationStyle(storm::solver::MultiplicationStyle::Regular);
            env.solver().multiplier().setType(storm::solver::MultiplierType::Gmmxx);
            return env;
        }
    };

    class SparseDoubleValueIterationNativeGaussSeidelMultEnvironment {
    public:
        static const SmgEngine engine = SmgEngine::PrismSparse;
        static const bool isExact = false;
        typedef double ValueType;
        typedef storm::models::sparse::Smg<ValueType> ModelType;
        static storm::Environment createEnvironment() {
            storm::Environment env;
            env.solver().minMax().setMethod(storm::solver::MinMaxMethod::ValueIteration);
            env.solver().minMax().setPrecision(storm::utility::convertNumber<storm::RationalNumber>(1e-10));
            env.solver().minMax().setMultiplicationStyle(storm::solver::MultiplicationStyle::GaussSeidel);
            env.solver().multiplier().setType(storm::solver::MultiplierType::Native);
            return env;
        }
    };

    class SparseDoubleValueIterationNativeRegularMultEnvironment {
    public:
        static const SmgEngine engine = SmgEngine::PrismSparse;
        static const bool isExact = false;
        typedef double ValueType;
        typedef storm::models::sparse::Smg<ValueType> ModelType;
        static storm::Environment createEnvironment() {
            storm::Environment env;
            env.solver().minMax().setMethod(storm::solver::MinMaxMethod::ValueIteration);
            env.solver().minMax().setPrecision(storm::utility::convertNumber<storm::RationalNumber>(1e-10));
            env.solver().minMax().setMultiplicationStyle(storm::solver::MultiplicationStyle::Regular);
            env.solver().multiplier().setType(storm::solver::MultiplierType::Native);
            return env;
        }
    };

    template<typename TestType>
    class SmgRpatlModelCheckerTest : public ::testing::Test {
    public:
        typedef typename TestType::ValueType ValueType;
        typedef typename storm::models::sparse::Smg<ValueType> SparseModelType;

        SmgRpatlModelCheckerTest() : _environment(TestType::createEnvironment()) {}
        storm::Environment const& env() const { return _environment; }
        ValueType parseNumber(std::string const& input) const { return storm::utility::convertNumber<ValueType>(input);}
        ValueType precision() const { return TestType::isExact ? parseNumber("0") : parseNumber("1e-6");}
        bool isSparseModel() const { return std::is_same<typename TestType::ModelType, SparseModelType>::value; }

        template <typename MT = typename TestType::ModelType>
                typename std::enable_if<std::is_same<MT, SparseModelType>::value, std::pair<std::shared_ptr<MT>, std::vector<std::shared_ptr<storm::logic::Formula const>>>>::type
                buildModelFormulas(std::string const& pathToPrismFile, std::string const& formulasAsString, std::string const& constantDefinitionString = "") const {
            std::pair<std::shared_ptr<MT>, std::vector<std::shared_ptr<storm::logic::Formula const>>> result;
            storm::prism::Program program = storm::api::parseProgram(pathToPrismFile);
            program = storm::utility::prism::preprocess(program, constantDefinitionString);
            if (TestType::engine == SmgEngine::PrismSparse) {
                result.second = storm::api::extractFormulasFromProperties(storm::api::parsePropertiesForPrismProgram(formulasAsString, program));
                result.first = storm::api::buildSparseModel<ValueType>(program, result.second)->template as<MT>();
            }
            return result;
        }

        std::vector<storm::modelchecker::CheckTask<storm::logic::Formula, ValueType>> getTasks(std::vector<std::shared_ptr<storm::logic::Formula const>> const& formulas) const {
            std::vector<storm::modelchecker::CheckTask<storm::logic::Formula, ValueType>> result;
            for (auto const& f : formulas) {
                result.emplace_back(*f);
            }
            return result;
        }

        template <typename MT = typename TestType::ModelType>
                typename std::enable_if<std::is_same<MT, SparseModelType>::value, std::shared_ptr<storm::modelchecker::AbstractModelChecker<MT>>>::type
                createModelChecker(std::shared_ptr<MT> const& model) const {
            if (TestType::engine == SmgEngine::PrismSparse) {
                return std::make_shared<storm::modelchecker::SparseSmgRpatlModelChecker<SparseModelType>>(*model);
            } else {
                STORM_LOG_ERROR("TestType::engine must be PrismSparse!");
                return nullptr;
            }
        }

        ValueType getQuantitativeResultAtInitialState(std::shared_ptr<storm::models::Model<ValueType>> const& model, std::unique_ptr<storm::modelchecker::CheckResult>& result) {
            auto filter = getInitialStateFilter(model);
            result->filter(*filter);
            return result->asQuantitativeCheckResult<ValueType>().getMin();
        }

    private:
        storm::Environment _environment;

        std::unique_ptr<storm::modelchecker::QualitativeCheckResult> getInitialStateFilter(std::shared_ptr<storm::models::Model<ValueType>> const& model) const {
            if (isSparseModel()) {
                return std::make_unique<storm::modelchecker::ExplicitQualitativeCheckResult>(model->template as<SparseModelType>()->getInitialStates());
            } else {
                STORM_LOG_ERROR("Smg Rpatl Model must be a Sparse Model!");
                return nullptr;
            }
        }
    };

    typedef ::testing::Types<
    SparseDoubleValueIterationGmmxxGaussSeidelMultEnvironment,
    SparseDoubleValueIterationGmmxxRegularMultEnvironment,
    SparseDoubleValueIterationNativeGaussSeidelMultEnvironment,
    SparseDoubleValueIterationNativeRegularMultEnvironment
    > TestingTypes;

    TYPED_TEST_SUITE(SmgRpatlModelCheckerTest, TestingTypes,);

    TYPED_TEST(SmgRpatlModelCheckerTest, Walker) {
        // NEXT tests
        std::string formulasString = "<<walker>> Pmax=? [X \"s2\"]";
        formulasString += "; <<walker>> Pmin=? [X \"s2\"]";
        formulasString += "; <<walker>> Pmax=? [X !\"s1\"]";
        formulasString += "; <<walker>> Pmin=? [X !\"s1\"]";
        // UNTIL tests
        formulasString += "; <<walker>> Pmax=? [ a=0 U a=1 ]";
        formulasString += "; <<walker>> Pmin=? [ a=0 U a=1 ]";
        formulasString += "; <<walker>> Pmax=? [ b=0 U b=1 ]";
        formulasString += "; <<walker>> Pmin=? [ b=0 U b=1 ]";
        // GLOBALLY tests
        formulasString += "; <<walker>> Pmax=? [G !\"s3\"]";
        formulasString += "; <<walker>> Pmin=? [G !\"s3\"]";
        formulasString += "; <<walker>> Pmax=? [G a=0 ]";
        formulasString += "; <<walker>> Pmin=? [G a=0 ]";
        // EVENTUALLY tests
        formulasString += "; <<walker>> Pmax=? [F \"s3\"]";
        formulasString += "; <<walker>> Pmin=? [F \"s3\"]";
        formulasString += "; <<walker>> Pmax=? [F [3,4] \"s4\"]";
        formulasString += "; <<walker>> Pmax=? [F [3,5] \"s4\"]";

        auto modelFormulas = this->buildModelFormulas(STORM_TEST_RESOURCES_DIR "/smg/walker.nm", formulasString);
        auto model = std::move(modelFormulas.first);
        auto tasks = this->getTasks(modelFormulas.second);
        EXPECT_EQ(5ul, model->getNumberOfStates());
        EXPECT_EQ(12ul, model->getNumberOfTransitions());
        ASSERT_EQ(model->getType(), storm::models::ModelType::Smg);
        auto checker = this->createModelChecker(model);
        std::unique_ptr<storm::modelchecker::CheckResult> result;

        // NEXT results
        result = checker->check(this->env(), tasks[0]);
        EXPECT_NEAR(this->parseNumber("0.6"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[1]);
        EXPECT_NEAR(this->parseNumber("0"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[2]);
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[3]);
        EXPECT_NEAR(this->parseNumber("0.6"), this->getQuantitativeResultAtInitialState(model, result), this->precision());

        // UNTIL results
        result = checker->check(this->env(), tasks[4]);
        EXPECT_NEAR(this->parseNumber("0.52"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[5]);
        EXPECT_NEAR(this->parseNumber("0"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[6]);
        EXPECT_NEAR(this->parseNumber("0.9999996417"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[7]);
        EXPECT_NEAR(this->parseNumber("0"), this->getQuantitativeResultAtInitialState(model, result), this->precision());

        // GLOBALLY tests
        result = checker->check(this->env(), tasks[8]);
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[9]);
        EXPECT_NEAR(this->parseNumber("0.65454565"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[10]);
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[11]);
        EXPECT_NEAR(this->parseNumber("0.48"), this->getQuantitativeResultAtInitialState(model, result), this->precision());

        // EVENTUALLY tests
        result = checker->check(this->env(), tasks[12]);
        EXPECT_NEAR(this->parseNumber("0.34545435"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[13]);
        EXPECT_NEAR(this->parseNumber("0"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[14]);
        EXPECT_NEAR(this->parseNumber("0.576"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[15]);
        EXPECT_NEAR(this->parseNumber("0.6336"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
    }

    TYPED_TEST(SmgRpatlModelCheckerTest, MessageHack) {
        // This test is for borders of bounded U with conversations from G and F
        // G
        std::string formulasString = "<<bob, alice>> Pmax=? [ G !\"hacked\" ]";

        // bounded F
        formulasString += "; <<bob, alice>> Pmin=? [ F \"hacked\" ]";
        formulasString += "; <<bob, alice>> Pmin=? [ F [1,2] \"hacked\" ]";
        formulasString += "; <<bob, alice>> Pmin=? [ F [3,16] \"hacked\" ]";
        formulasString += "; <<bob, alice>> Pmin=? [ F [0,17] \"hacked\" ]";
        formulasString += "; <<bob, alice>> Pmin=? [ F [17,31] \"hacked\" ]";
        formulasString += "; <<bob, alice>> Pmin=? [ F [17,32] \"hacked\" ]";

        auto modelFormulas = this->buildModelFormulas(STORM_TEST_RESOURCES_DIR "/smg/messageHack.nm", formulasString);
        auto model = std::move(modelFormulas.first);
        auto tasks = this->getTasks(modelFormulas.second);
        EXPECT_EQ(30ul, model->getNumberOfStates());
        EXPECT_EQ(31ul, model->getNumberOfTransitions());
        ASSERT_EQ(model->getType(), storm::models::ModelType::Smg);
        auto checker = this->createModelChecker(model);
        std::unique_ptr<storm::modelchecker::CheckResult> result;

        // G results
        result = checker->check(this->env(), tasks[0]);
        EXPECT_NEAR(this->parseNumber("1.99379598e-05"), this->getQuantitativeResultAtInitialState(model, result), this->precision());

        // bounded F results
        result = checker->check(this->env(), tasks[1]);
        EXPECT_NEAR(this->parseNumber("0.999980062"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[2]);
        EXPECT_NEAR(this->parseNumber("0.05"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[3]);
        EXPECT_NEAR(this->parseNumber("0.05"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[4]);
        EXPECT_NEAR(this->parseNumber("0.0975"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[5]);
        EXPECT_NEAR(this->parseNumber("0.0975"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[6]);
        EXPECT_NEAR(this->parseNumber("0.142625"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
    }

    TYPED_TEST(SmgRpatlModelCheckerTest, RightDecision) {
        // This test is for making decisions
        // testing probabilities for decisions
        std::string formulasString = "<<hiker>> Pmax=? [ F <=3 \"target\" ]";
        formulasString += "; <<hiker>> Pmax=? [ F <=5 \"target\" ]";
        formulasString += "; <<hiker, native>> Pmax=? [ F <=3 \"target\" ]";
        formulasString += "; <<hiker>> Pmin=? [ F \"target\" ]";

        auto modelFormulas = this->buildModelFormulas(STORM_TEST_RESOURCES_DIR "/smg/rightDecision.nm", formulasString);
        auto model = std::move(modelFormulas.first);
        auto tasks = this->getTasks(modelFormulas.second);
        EXPECT_EQ(11ul, model->getNumberOfStates());
        EXPECT_EQ(15ul, model->getNumberOfTransitions());
        ASSERT_EQ(model->getType(), storm::models::ModelType::Smg);
        auto checker = this->createModelChecker(model);
        std::unique_ptr<storm::modelchecker::CheckResult> result;

        // probability results
        result = checker->check(this->env(), tasks[0]);
        EXPECT_NEAR(this->parseNumber("0.9"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[1]);
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[2]);
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[3]);
        EXPECT_NEAR(this->parseNumber("0"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
    }

    TYPED_TEST(SmgRpatlModelCheckerTest, RobotCircle) {
        // This test is for testing bounded globally with upper bound and in an interval (with upper and lower bound)
        std::string formulasString = " <<friendlyRobot>> Pmax=? [ G<1 !\"crash\" ]";
        formulasString += "; <<friendlyRobot>> Pmax=? [ G<=1 !\"crash\" ]";
        formulasString += "; <<friendlyRobot>> Pmax=? [ G<=5 !\"crash\" ]";
        formulasString += "; <<friendlyRobot>> Pmax=? [ G<=6 !\"crash\" ]";
        formulasString += "; <<friendlyRobot>> Pmax=? [ G<=7 !\"crash\" ]";
        formulasString += "; <<friendlyRobot>> Pmax=? [ G<=8 !\"crash\" ]";

        formulasString += "; <<friendlyRobot>> Pmax=? [ G[1,5] !\"crash\" ]";
        formulasString += "; <<friendlyRobot>> Pmax=? [ G[5,6] !\"crash\" ]";
        formulasString += "; <<friendlyRobot>> Pmax=? [ G[7,8] !\"crash\" ]";

        auto modelFormulas = this->buildModelFormulas(STORM_TEST_RESOURCES_DIR "/smg/robotCircle.nm", formulasString);
        auto model = std::move(modelFormulas.first);
        auto tasks = this->getTasks(modelFormulas.second);
        EXPECT_EQ(81ul, model->getNumberOfStates());
        EXPECT_EQ(196ul, model->getNumberOfTransitions());
        EXPECT_EQ(148ul, model->getNumberOfChoices());
        ASSERT_EQ(model->getType(), storm::models::ModelType::Smg);
        auto checker = this->createModelChecker(model);
        std::unique_ptr<storm::modelchecker::CheckResult> result;

        // results for bounded globally with upper bound
        result = checker->check(this->env(), tasks[0]);
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[1]);
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[2]);
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[3]);
        EXPECT_NEAR(this->parseNumber("0.975"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[4]);
        EXPECT_NEAR(this->parseNumber("0.975"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[5]);
        EXPECT_NEAR(this->parseNumber("0.975"), this->getQuantitativeResultAtInitialState(model, result), this->precision());

        // results for bounded globally with upper and lower bound
        result = checker->check(this->env(), tasks[6]);
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[7]);
        EXPECT_NEAR(this->parseNumber("0.975"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[8]);
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
    }

    // TODO: create more test cases (files)
}
