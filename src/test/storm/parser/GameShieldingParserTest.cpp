#include "test/storm_gtest.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/logic/FragmentSpecification.h"
#include "storm/storage/expressions/ExpressionManager.h"

TEST(GameShieldingParserTest, PreSafetyShieldTest) {
    storm::parser::FormulaParser formulaParser;

    std::string filename = "preSafetyShieldFileName";
    std::string value = "0.9";
    std::string input = "<" + filename + ", PreSafety, lambda=" + value + "> <<p1,p2>> Pmax=? [F \"label\"]";

    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());

    std::vector<storm::jani::Property> property;
    ASSERT_NO_THROW(property = formulaParser.parseFromString(input));
    EXPECT_TRUE(property.at(0).isShieldingProperty());

    std::shared_ptr<storm::logic::ShieldExpression const> shieldExpression(nullptr);
    ASSERT_NO_THROW(shieldExpression = property.at(0).getShieldingExpression());
    EXPECT_TRUE(shieldExpression->isPreSafetyShield());
    EXPECT_FALSE(shieldExpression->isPostSafetyShield());
    EXPECT_FALSE(shieldExpression->isOptimalShield());
    EXPECT_TRUE(shieldExpression->isRelative());
    EXPECT_EQ(std::stod(value), shieldExpression->getValue());
    EXPECT_EQ(filename, shieldExpression->getFilename());
}

TEST(GameShieldingParserTest, PostShieldTest) {
    storm::parser::FormulaParser formulaParser;

    std::string filename = "postSafetyShieldFileName";
    std::string value = "0.7569";
    std::string input = "<" + filename + ", PostSafety, gamma=" + value + "> <<p1,p2,p3>> Pmin=? [X !\"label\"]";

    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());

    std::vector<storm::jani::Property> property;
    ASSERT_NO_THROW(property = formulaParser.parseFromString(input));
    EXPECT_TRUE(property.at(0).isShieldingProperty());

    std::shared_ptr<storm::logic::ShieldExpression const> shieldExpression(nullptr);
    ASSERT_NO_THROW(shieldExpression = property.at(0).getShieldingExpression());
    EXPECT_FALSE(shieldExpression->isPreSafetyShield());
    EXPECT_TRUE(shieldExpression->isPostSafetyShield());
    EXPECT_FALSE(shieldExpression->isOptimalShield());
    EXPECT_FALSE(shieldExpression->isRelative());
    EXPECT_EQ(std::stod(value), shieldExpression->getValue());
    EXPECT_EQ(filename, shieldExpression->getFilename());
}

TEST(GameShieldingParserTest, OptimalShieldTest) {
    std::shared_ptr<storm::expressions::ExpressionManager> manager(new storm::expressions::ExpressionManager());
    manager->declareIntegerVariable("x");
    manager->declareIntegerVariable("y");

    storm::parser::FormulaParser formulaParser(manager);

    std::string filename = "optimalShieldFileName";
    std::string input = "<" + filename + ", Optimal> <<p1,p2,p3>> Pmax=? [G x>y]";

    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());

    std::vector<storm::jani::Property> property;
    ASSERT_NO_THROW(property = formulaParser.parseFromString(input));
    EXPECT_TRUE(property.at(0).isShieldingProperty());

    std::shared_ptr<storm::logic::ShieldExpression const> shieldExpression(nullptr);
    ASSERT_NO_THROW(shieldExpression = property.at(0).getShieldingExpression());
    EXPECT_FALSE(shieldExpression->isPreSafetyShield());
    EXPECT_FALSE(shieldExpression->isPostSafetyShield());
    EXPECT_TRUE(shieldExpression->isOptimalShield());
    EXPECT_EQ(filename, shieldExpression->getFilename());
}
