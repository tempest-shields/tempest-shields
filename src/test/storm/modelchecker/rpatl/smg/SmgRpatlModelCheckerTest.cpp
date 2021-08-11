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

/*        bool getQualitativeResultAtInitialState(std::shared_ptr<storm::models::Model<ValueType>> const& model, std::unique_ptr<storm::modelchecker::CheckResult>& result) {
            auto filter = getInitialStateFilter(model);
            result->filter(*filter);
            return result->asQualitativeCheckResult().forallTrue();
        }*/

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
        result = checker->check(this->env(), tasks[0]); //OK
        EXPECT_NEAR(this->parseNumber("0.6"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[1]); //OK
        EXPECT_NEAR(this->parseNumber("0"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[2]); //OK
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[3]); //OK
        EXPECT_NEAR(this->parseNumber("0.6"), this->getQuantitativeResultAtInitialState(model, result), this->precision());

        // UNTIL results
        result = checker->check(this->env(), tasks[4]); //FAIL -> 0.99999922964892118 this may happen if the walker does not get caught in s4
        EXPECT_NEAR(this->parseNumber("0.52"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[5]); //OK
        EXPECT_NEAR(this->parseNumber("0"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[6]); //OK
        EXPECT_NEAR(this->parseNumber("0.9999996417"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[7]); //OK
        EXPECT_NEAR(this->parseNumber("0"), this->getQuantitativeResultAtInitialState(model, result), this->precision());

        // GLOBALLY tests
        result = checker->check(this->env(), tasks[8]); //FAIL -> 2.686012256170045e-06 this may happen if the walker does not stay in s0
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[9]); //FAIL -> 1 this is maybe exactly the other way round (by minimizing leaving)
        EXPECT_NEAR(this->parseNumber("0.65454565"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[10]); //FAIL -> 7.7035107881595621e-07
        EXPECT_NEAR(this->parseNumber("1"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[11]); //FAIL -> 1
        EXPECT_NEAR(this->parseNumber("0.48"), this->getQuantitativeResultAtInitialState(model, result), this->precision());

        // EVENTUALLY tests
        result = checker->check(this->env(), tasks[12]); //FAIL -> 0.99999731398774383
        EXPECT_NEAR(this->parseNumber("0.34545435"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[13]); //OK
        EXPECT_NEAR(this->parseNumber("0"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[14]); //FAIL -> 0.63359999999999994 here is maybe an offset of 1 step
        EXPECT_NEAR(this->parseNumber("0.576"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
        result = checker->check(this->env(), tasks[15]); //FAIL -> 0.64511999999999992
        EXPECT_NEAR(this->parseNumber("0.6336"), this->getQuantitativeResultAtInitialState(model, result), this->precision());
    }

    // TODO: create more test cases (files)
}
