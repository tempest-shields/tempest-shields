#include "test/storm_gtest.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/logic/FragmentSpecification.h"
#include "storm/storage/expressions/ExpressionManager.h"

TEST(GameFormulaParserTest, LabelTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "<<p1>> Pmax=? [F \"label\"]";

    std::shared_ptr<storm::logic::Formula const> gameFormula(nullptr);
    ASSERT_NO_THROW(gameFormula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(gameFormula->isStateFormula());
    EXPECT_TRUE(gameFormula->isUnaryStateFormula());
    EXPECT_TRUE(gameFormula->isGameFormula());
}

TEST(GameFormulaParserTest, ComplexLabelTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "<<p1,p2>> Pmin=? [X !(\"a\" & \"b\") | \"a\" & !\"c\"]";

    std::shared_ptr<storm::logic::Formula const> gameFormula(nullptr);
    ASSERT_NO_THROW(gameFormula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(gameFormula->isStateFormula());
    EXPECT_TRUE(gameFormula->isUnaryStateFormula());
    EXPECT_TRUE(gameFormula->isGameFormula());
}

TEST(GameFormulaParserTest, ExpressionTest) {
    std::shared_ptr<storm::expressions::ExpressionManager> manager(new storm::expressions::ExpressionManager());
    manager->declareBooleanVariable("x");
    manager->declareIntegerVariable("y");

    storm::parser::FormulaParser formulaParser(manager);

    std::string input = "<<p1, p2, p3>> Pmin=? [G !(x | y > 3)]";

    std::shared_ptr<storm::logic::Formula const> gameFormula(nullptr);
    ASSERT_NO_THROW(gameFormula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(gameFormula->isStateFormula());
    EXPECT_TRUE(gameFormula->isUnaryStateFormula());
    EXPECT_TRUE(gameFormula->isGameFormula());
}

TEST(GameFormulaParserTest, LabelAndExpressionTest) {
    std::shared_ptr<storm::expressions::ExpressionManager> manager(new storm::expressions::ExpressionManager());
    manager->declareBooleanVariable("x");
    manager->declareIntegerVariable("y");

    storm::parser::FormulaParser formulaParser(manager);

    std::string input = "<<p1, p2, p3>> Pmax=? [\"b\" U !\"a\" | x | y > 3]";
    std::shared_ptr<storm::logic::Formula const> gameFormula(nullptr);
    ASSERT_NO_THROW(gameFormula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(gameFormula->isStateFormula());
    EXPECT_TRUE(gameFormula->isUnaryStateFormula());
    EXPECT_TRUE(gameFormula->isGameFormula());

    input = "<<p1, p2, p3>> Pmax=? [x | y > 3 | !\"a\" U \"b\"]";
    ASSERT_NO_THROW(gameFormula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(gameFormula->isStateFormula());
    EXPECT_TRUE(gameFormula->isUnaryStateFormula());
    EXPECT_TRUE(gameFormula->isGameFormula());
}

TEST(GameFormulaParserTest, OnePlayerCoalitionTest) {
    storm::parser::FormulaParser formulaParser;

    std::string player = "p1";
    std::string input = "<<" + player + ">>" + " Pmax=? [F \"label\"]";

    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));

    auto const& gameFormula = formula->asGameFormula();
    auto const& playerCoalition = gameFormula.getCoalition();
    //ASSERT_NO_THROW(auto const& players = playerCoalition.getPlayers());

    std::ostringstream stream;
    stream << playerCoalition;
    std::string playerCoalitionString = stream.str();

    EXPECT_EQ(player, playerCoalitionString);
}

TEST(GameFormulaParserTest, PlayersCoalitionTest) {
    storm::parser::FormulaParser formulaParser;

    std::string player = "p1, p2,  p3,   p4,     p5";
    std::string input = "<<" + player + ">>" + " Pmin=? [X \"label\"]";

    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));

    auto const& gameFormula = formula->asGameFormula();
    auto const& playerCoalition = gameFormula.getCoalition();
    EXPECT_EQ(5ul, playerCoalition.getPlayers().size());

    std::ostringstream stream;
    stream << playerCoalition;
    std::string playerCoalitionString = stream.str();

    std::string playerWithoutWhiteSpace = "p1,p2,p3,p4,p5";

    EXPECT_EQ(playerWithoutWhiteSpace, playerCoalitionString);
}

TEST(GameFormulaParserTest, ProbabilityOperatorTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "<<player>> Pmax=? [\"a\" U \"b\"]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));

    auto const& gameFormula = formula->asGameFormula();
    auto const& probFormula = gameFormula.getSubformula();

    EXPECT_TRUE(probFormula.isProbabilityOperatorFormula());
    EXPECT_FALSE(probFormula.asProbabilityOperatorFormula().hasBound());
    EXPECT_TRUE(probFormula.asProbabilityOperatorFormula().hasOptimalityType());
}

// TODO: NextOperatorTest

TEST(GameFormulaParserTest, UntilOperatorTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "<<p>> Pmax=? [\"a\" U \"b\"]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    auto const& gameFormula1 = formula->asGameFormula();
    auto const& probFormula1 = gameFormula1.getSubformula();
    auto const& rawFormula1 = probFormula1.asProbabilityOperatorFormula().getSubformula();
    EXPECT_TRUE(rawFormula1.isUntilFormula());
    EXPECT_FALSE(rawFormula1.isBoundedUntilFormula());

    input = "<<p>> Pmax=? [\"a\" U <= 4 \"b\"]";
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    auto const& gameFormula2 = formula->asGameFormula();
    auto const& probFormula2 = gameFormula2.getSubformula();
    auto const& rawFormula2 = probFormula2.asProbabilityOperatorFormula().getSubformula();
    EXPECT_TRUE(rawFormula2.isBoundedUntilFormula());
    EXPECT_TRUE(rawFormula2.asBoundedUntilFormula().getTimeBoundReference().isTimeBound());
    EXPECT_EQ(0, rawFormula2.asBoundedUntilFormula().getLowerBound().evaluateAsInt());
    EXPECT_EQ(4, rawFormula2.asBoundedUntilFormula().getUpperBound().evaluateAsInt());

    input = "<<p1,p2>> Pmin=? [ (a&b) U [5,9] (b|c) ]";
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    auto const& gameFormula3 = formula->asGameFormula();
    auto const& probFormula3 = gameFormula3.getSubformula();
    auto const& rawFormula3 = probFormula3.asProbabilityOperatorFormula().getSubformula();
    EXPECT_TRUE(rawFormula3.isBoundedUntilFormula());
    EXPECT_TRUE(rawFormula3.asBoundedUntilFormula().getTimeBoundReference().isTimeBound());
    EXPECT_EQ(5, rawFormula3.asBoundedUntilFormula().getLowerBound().evaluateAsInt());
    EXPECT_EQ(9, rawFormula3.asBoundedUntilFormula().getUpperBound().evaluateAsInt());
}


