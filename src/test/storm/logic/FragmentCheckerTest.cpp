#include "test/storm_gtest.h"
#include "storm-config.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/logic/FragmentChecker.h"
#include "storm/exceptions/WrongFormatException.h"
#include "storm/storage/expressions/ExpressionManager.h"

TEST(FragmentCheckerTest, Propositional) {
    auto expManager = std::make_shared<storm::expressions::ExpressionManager>();
    storm::logic::FragmentChecker checker;
    storm::logic::FragmentSpecification prop = storm::logic::propositional();

    storm::parser::FormulaParser formulaParser(expManager);
    std::shared_ptr<storm::logic::Formula const> formula;

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("\"label\""));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prop));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("true"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prop));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("true | \"label\""));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prop));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("!true"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prop));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("true"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prop));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F true]"));
    EXPECT_FALSE(checker.conformsToSpecification(*formula, prop));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("false | P>0.5 [G \"label\"]"));
    EXPECT_FALSE(checker.conformsToSpecification(*formula, prop));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F \"label\"]"));
    EXPECT_FALSE(checker.conformsToSpecification(*formula, prop));
}

TEST(FragmentCheckerTest, Pctl) {
    auto expManager = std::make_shared<storm::expressions::ExpressionManager>();
    storm::logic::FragmentChecker checker;
    storm::logic::FragmentSpecification pctl = storm::logic::pctl();

    storm::parser::FormulaParser formulaParser(expManager);
    std::shared_ptr<storm::logic::Formula const> formula;

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("\"label\""));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, pctl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, pctl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F P=? [F \"label\"]]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, pctl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("R=? [F \"label\"]"));
    EXPECT_FALSE(checker.conformsToSpecification(*formula, pctl));
}

TEST(FragmentCheckerTest, Prctl) {
    auto expManager = std::make_shared<storm::expressions::ExpressionManager>();
    storm::logic::FragmentChecker checker;
    storm::logic::FragmentSpecification prctl = storm::logic::prctl();

    storm::parser::FormulaParser formulaParser(expManager);
    std::shared_ptr<storm::logic::Formula const> formula;

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("\"label\""));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prctl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prctl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F P=? [F \"label\"]]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prctl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("R=? [F \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prctl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("R=? [C<=3]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prctl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F[0,1] \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prctl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("<shield, PostSafety, gamma=0.678> Pmax=? [\"label1\" U [5,7] \"label2\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prctl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("<shield, PreSafety, lambda=0.9> Pmax=? [G \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prctl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("<shieldFileName, Optimal> Pmin=? [F <5 \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, prctl));
}

TEST(FragmentCheckerTest, Csl) {
    auto expManager = std::make_shared<storm::expressions::ExpressionManager>();
    storm::logic::FragmentChecker checker;
    storm::logic::FragmentSpecification csl = storm::logic::csl();

    storm::parser::FormulaParser formulaParser(expManager);
    std::shared_ptr<storm::logic::Formula const> formula;

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("\"label\""));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, csl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, csl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F P=? [F \"label\"]]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, csl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("R=? [F \"label\"]"));
    EXPECT_FALSE(checker.conformsToSpecification(*formula, csl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F[0,1] \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, csl));
}

TEST(FragmentCheckerTest, Csrl) {
    auto expManager = std::make_shared<storm::expressions::ExpressionManager>();
    storm::logic::FragmentChecker checker;
    storm::logic::FragmentSpecification csrl = storm::logic::csrl();

    storm::parser::FormulaParser formulaParser(expManager);
    std::shared_ptr<storm::logic::Formula const> formula;

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("\"label\""));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, csrl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, csrl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F P=? [F \"label\"]]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, csrl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("R=? [F \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, csrl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("R=? [C<=3]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, csrl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F[0,1] \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, csrl));
}

TEST(FragmentCheckerTest, MultiObjective) {
    storm::logic::FragmentChecker checker;
    storm::logic::FragmentSpecification multiobjective = storm::logic::multiObjective();

    storm::parser::FormulaParser formulaParser;
    std::shared_ptr<storm::logic::Formula const> formula;

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("\"label\""));
    EXPECT_FALSE(checker.conformsToSpecification(*formula, multiobjective));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("P=? [F \"label\"]"));
    EXPECT_FALSE(checker.conformsToSpecification(*formula, multiobjective));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("multi(R<0.3 [ C ], P<0.6 [(F \"label1\") & G \"label2\"])"));
    EXPECT_FALSE(checker.conformsToSpecification(*formula, multiobjective));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("Pmax=? [ F multi(R<0.3 [ C ], P<0.6 [F \"label\" & \"label\" & R<=4[F \"label\"]])]"));
    EXPECT_FALSE(checker.conformsToSpecification(*formula, multiobjective));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("multi(R<0.3 [ C ], P<0.6 [F \"label\"], R<=4[F \"label\"])"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, multiobjective));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("multi(R<0.3 [ C<=3 ], P<0.6 [F \"label\"], R<=4[F \"label\"])"));
    EXPECT_FALSE(checker.conformsToSpecification(*formula, multiobjective));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("multi(R<0.3 [ C ], P<0.6 [F \"label\" & \"otherlabel\"], P<=4[\"label\" U<=42 \"otherlabel\"])"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, multiobjective));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("multi(P=? [F P<0.5 [F \"label\"]], R<0.3 [ C ] )"));
    EXPECT_FALSE(checker.conformsToSpecification(*formula, multiobjective));
}


TEST(FragmentCheckerTest, Rpatl) {
    auto expManager = std::make_shared<storm::expressions::ExpressionManager>();
    storm::logic::FragmentChecker checker;
    storm::logic::FragmentSpecification rpatl = storm::logic::rpatl();

    storm::parser::FormulaParser formulaParser(expManager);
    std::shared_ptr<storm::logic::Formula const> formula;

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("<<p1>> P=? [F \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, rpatl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("<<1,2>> Pmin=? [ \"label1\" U \"label2\" ]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, rpatl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("<<p1, p2>> Rmax=? [ LRA ]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, rpatl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("<<player1>> R=? [C<=3]"));
    EXPECT_FALSE(checker.conformsToSpecification(*formula, rpatl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("<<1,2,3>> Pmin=? [F [2,5] \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, rpatl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("<shield, PostSafety, gamma=0.749> <<1,2,3,4,5>> Pmax=? [\"label1\" U [0,7] \"label2\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, rpatl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("<shield, PreSafety, lambda=0.749> <<a,b,c>> Pmax=? [G \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, rpatl));

    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString("<shieldFileName, Optimal> <<p1,p2>> Pmin=? [G \"label\"]"));
    EXPECT_TRUE(checker.conformsToSpecification(*formula, rpatl));

}
