#include "test/storm_gtest.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/logic/FragmentSpecification.h"
#include "storm/storage/expressions/ExpressionManager.h"

TEST(MdpShieldingParserTest, PreSafetyShieldTest) {
    storm::parser::FormulaParser formulaParser;

    std::string filename = "preSafetyShieldFileName";
    std::string value = "0.6667";
    std::string input = "<" + filename + ", PreSafety, gamma=" + value + "> Pmax=? [F \"label\"]";

    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    std::vector<storm::jani::Property> property;
    ASSERT_NO_THROW(property = formulaParser.parseFromString(input));
    EXPECT_TRUE(property.at(0).isShieldingProperty());

    std::shared_ptr<storm::logic::ShieldExpression const> shieldExpression(nullptr);
    ASSERT_NO_THROW(shieldExpression = property.at(0).getShieldingExpression());
    EXPECT_TRUE(shieldExpression->isPreSafetyShield());
    EXPECT_FALSE(shieldExpression->isPostSafetyShield());
    EXPECT_FALSE(shieldExpression->isOptimalShield());
    EXPECT_FALSE(shieldExpression->isRelative());
    EXPECT_EQ(std::stod(value), shieldExpression->getValue());
    EXPECT_EQ(filename, shieldExpression->getFilename());
}

TEST(MdpShieldingParserTest, PostShieldTest) {
    storm::parser::FormulaParser formulaParser;

    std::string filename = "postSafetyShieldFileName";
    std::string value = "0.95";
    std::string input = "<" + filename + ", PostSafety, lambda=" + value + "> Pmin=? [X !\"label\"]";

    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    std::vector<storm::jani::Property> property;
    ASSERT_NO_THROW(property = formulaParser.parseFromString(input));
    EXPECT_TRUE(property.at(0).isShieldingProperty());

    std::shared_ptr<storm::logic::ShieldExpression const> shieldExpression(nullptr);
    ASSERT_NO_THROW(shieldExpression = property.at(0).getShieldingExpression());
    EXPECT_FALSE(shieldExpression->isPreSafetyShield());
    EXPECT_TRUE(shieldExpression->isPostSafetyShield());
    EXPECT_FALSE(shieldExpression->isOptimalShield());
    EXPECT_TRUE(shieldExpression->isRelative());
    EXPECT_EQ(std::stod(value), shieldExpression->getValue());
    EXPECT_EQ(filename, shieldExpression->getFilename());
}

TEST(MdpShieldingParserTest, OptimalShieldTest) {
    std::shared_ptr<storm::expressions::ExpressionManager> manager(new storm::expressions::ExpressionManager());
    manager->declareBooleanVariable("a");
    manager->declareIntegerVariable("x");

    storm::parser::FormulaParser formulaParser(manager);

    std::string filename = "optimalShieldFileName";
    std::string input = "<" + filename + ", Optimal> Pmax=? [G (a|x>3)]";

    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
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
