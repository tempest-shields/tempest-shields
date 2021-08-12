#include "test/storm_gtest.h"
#include "storm-config.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/logic/FragmentSpecification.h"
#include "storm/exceptions/WrongFormatException.h"
#include "storm/storage/expressions/ExpressionManager.h"

// TODO: write this to <<player>> inputs (games)!

TEST(GameFormulaParserTest, LabelTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "<<player>> \"label\"";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(formula->isAtomicLabelFormula());
}

TEST(GameFormulaParserTest, ComplexLabelTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "!(\"a\" & \"b\") | \"a\" & !\"c\"";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(formula->isInFragment(storm::logic::propositional()));
    EXPECT_TRUE(formula->isBinaryBooleanStateFormula());
}

TEST(GameFormulaParserTest, ExpressionTest) {
    std::shared_ptr<storm::expressions::ExpressionManager> manager(new storm::expressions::ExpressionManager());
    manager->declareBooleanVariable("x");
    manager->declareIntegerVariable("y");

    storm::parser::FormulaParser formulaParser(manager);

    std::string input = "!(x | y > 3)";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(formula->isInFragment(storm::logic::propositional()));
    EXPECT_TRUE(formula->isUnaryBooleanStateFormula());
}

TEST(GameFormulaParserTest, LabelAndExpressionTest) {
    std::shared_ptr<storm::expressions::ExpressionManager> manager(new storm::expressions::ExpressionManager());
    manager->declareBooleanVariable("x");
    manager->declareIntegerVariable("y");

    storm::parser::FormulaParser formulaParser(manager);

    std::string input = "!\"a\" | x | y > 3";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(formula->isInFragment(storm::logic::propositional()));

    input = "x | y > 3 | !\"a\"";
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isInFragment(storm::logic::propositional()));
}