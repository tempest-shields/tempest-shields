#include "test/storm_gtest.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/logic/FragmentSpecification.h"
#include "storm/storage/expressions/ExpressionManager.h"

TEST(GameFormulaParserTest, OnePlayerCoalitionTest) {
    storm::parser::FormulaParser formulaParser;

    std::string player = "p1";
    std::string input = "<<" + player + ">>" + " Pmax=? [F \"label\"]";

    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(formula->isGameFormula());
    auto const& playerCoalition = formula->asGameFormula().getCoalition();
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

    EXPECT_TRUE(formula->isGameFormula());
    auto const& playerCoalition = formula->asGameFormula().getCoalition();
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

    EXPECT_TRUE(formula->isGameFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isInFragment(storm::logic::rpatl()));
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isProbabilityOperatorFormula());
    EXPECT_FALSE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().hasBound());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().hasOptimalityType());
}

TEST(GameFormulaParserTest, NextOperatorTest) {
    std::shared_ptr<storm::expressions::ExpressionManager> manager(new storm::expressions::ExpressionManager());
    manager->declareBooleanVariable("x");
    manager->declareIntegerVariable("a");
    storm::parser::FormulaParser formulaParser(manager);

    std::string input = "<<p>> Pmax=? [X \"a\"]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isInFragment(storm::logic::rpatl()));
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isProbabilityOperatorFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().isNextFormula());

    input = "<<p1, p2, p3>> Pmin=?  [X !x ]";
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isInFragment(storm::logic::rpatl()));
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isProbabilityOperatorFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().isNextFormula());

    input = "<<player1, player2, 3,4,5>> Pmax=? [ X a>5 ]";
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isInFragment(storm::logic::rpatl()));
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isProbabilityOperatorFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().isNextFormula());
}

TEST(GameFormulaParserTest, UntilOperatorTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "<<p>> Pmax=? [\"a\" U \"b\"]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isInFragment(storm::logic::rpatl()));
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isProbabilityOperatorFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().isUntilFormula());
    EXPECT_FALSE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().isBoundedUntilFormula());

    input = "<<p>> Pmax=? [ \"a\" U <= 4 \"b\" ]";
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isInFragment(storm::logic::rpatl()));
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isProbabilityOperatorFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().isBoundedUntilFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().asBoundedUntilFormula().getTimeBoundReference().isTimeBound());
    EXPECT_FALSE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().asBoundedUntilFormula().hasLowerBound());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().asBoundedUntilFormula().hasUpperBound());
    EXPECT_EQ(4, formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().asBoundedUntilFormula().getUpperBound().evaluateAsInt());

    input = "<<p1, p2>> Pmin=?  [ \"a\" & \"b\" U [5,9] \"c\" ] ";
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isInFragment(storm::logic::rpatl()));
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isProbabilityOperatorFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().isBoundedUntilFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().asBoundedUntilFormula().getTimeBoundReference().isTimeBound());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().asBoundedUntilFormula().hasLowerBound());
    EXPECT_EQ(5, formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().asBoundedUntilFormula().getLowerBound().evaluateAsInt());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().asBoundedUntilFormula().hasUpperBound());
    EXPECT_EQ(9, formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().asBoundedUntilFormula().getUpperBound().evaluateAsInt());
}

TEST(GameFormulaParserTest, RewardOperatorTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "<<p1, p2>> Rmin<0.9 [F \"a\"]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isRewardOperatorFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asRewardOperatorFormula().hasBound());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asRewardOperatorFormula().hasOptimalityType());
    EXPECT_FALSE(formula->asGameFormula().getSubformula().isInFragment(storm::logic::rpatl()));

    input = "<<p1, p2>> R=? [I=10]";
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isRewardOperatorFormula());
    EXPECT_FALSE(formula->asGameFormula().getSubformula().asRewardOperatorFormula().hasBound());
    EXPECT_FALSE(formula->asGameFormula().getSubformula().asRewardOperatorFormula().hasOptimalityType());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().asRewardOperatorFormula().getSubformula().isInstantaneousRewardFormula());
    EXPECT_FALSE(formula->asGameFormula().getSubformula().isInFragment(storm::logic::rpatl()));
}

TEST(GameFormulaParserTest, ConditionalProbabilityTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "<<p1, p2>> P<0.9 [F \"a\" || F \"b\"]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isProbabilityOperatorFormula());
    storm::logic::ProbabilityOperatorFormula const& probFormula = formula->asGameFormula().getSubformula().asProbabilityOperatorFormula();
    EXPECT_TRUE(probFormula.getSubformula().isConditionalProbabilityFormula());
    EXPECT_FALSE(formula->asGameFormula().getSubformula().isInFragment(storm::logic::rpatl()));
}

TEST(GameFormulaParserTest, NestedPathFormulaTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "<<p1, p2, p3>> P<0.9 [F X \"a\"]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isProbabilityOperatorFormula());
    ASSERT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().isEventuallyFormula());
    ASSERT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().asEventuallyFormula().getSubformula().isNextFormula());
    EXPECT_FALSE(formula->asGameFormula().getSubformula().isInFragment(storm::logic::rpatl()));
}

TEST(GameFormulaParserTest, CommentTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "// This is a comment. And this is a commented out formula: <<p>> P<=0.5 [ F \"a\" ] The next line contains the actual formula. \n<<p>> P<=0.5 [ X \"b\" ] // Another comment \n // And again: another comment.";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isGameFormula());
    EXPECT_TRUE(formula->asGameFormula().getSubformula().isProbabilityOperatorFormula());
    ASSERT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().isNextFormula());
    ASSERT_TRUE(formula->asGameFormula().getSubformula().asProbabilityOperatorFormula().getSubformula().asNextFormula().getSubformula().isAtomicLabelFormula());
}

TEST(GameFormulaParserTest, WrongFormatTest) {
    std::shared_ptr<storm::expressions::ExpressionManager> manager(new storm::expressions::ExpressionManager());
    manager->declareBooleanVariable("x");
    manager->declareIntegerVariable("y");

    storm::parser::FormulaParser formulaParser(manager);
    std::string input = "<<p1,p2>> P>0.5 [ a ]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    STORM_SILENT_EXPECT_THROW(formula = formulaParser.parseSingleFormulaFromString(input), storm::exceptions::WrongFormatException);

    input = "<<p>> P=0.5 [F \"a\"]";
    STORM_SILENT_EXPECT_THROW(formula = formulaParser.parseSingleFormulaFromString(input), storm::exceptions::WrongFormatException);

    input = "<<p1, p2>> P>0.5 [F !(x = 0)]";
    STORM_SILENT_EXPECT_THROW(formula = formulaParser.parseSingleFormulaFromString(input), storm::exceptions::WrongFormatException);

    input = "<< p1, p2 >> P>0.5 [F !y]";
    STORM_SILENT_EXPECT_THROW(formula = formulaParser.parseSingleFormulaFromString(input), storm::exceptions::WrongFormatException);

    input = "<< 1,2,3 >> P>0.5 [F y!=0)]";
    STORM_SILENT_EXPECT_THROW(formula = formulaParser.parseSingleFormulaFromString(input), storm::exceptions::WrongFormatException);
}
