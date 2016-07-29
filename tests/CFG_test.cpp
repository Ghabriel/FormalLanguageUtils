/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <gtest/gtest.h>
#include "CFG.hpp"
#include "DidacticNotation.hpp"

using set = std::unordered_set<std::string>;

class TestCFG : public ::testing::Test {
protected:
    CFG cfg;
};

TEST_F(TestCFG, SymbolLists) {
    ASSERT_NO_THROW(cfg.add("<S>", "a<A>b", ""));
    ASSERT_NO_THROW(cfg.add("<A>", "a<A>", "b<A>", ""));
    EXPECT_EQ(set({"a", "b"}), cfg.getTerminals());
    EXPECT_EQ(set({"<S>", "<A>"}), cfg.getNonTerminals());
}

TEST_F(TestCFG, Consistency) {
    EXPECT_TRUE(cfg.isConsistent());
    cfg.add("<S>", "a<A>b", "");
    EXPECT_FALSE(cfg.isConsistent());
    cfg.add("<A>", "a<A>", "b<A>", "");
    EXPECT_TRUE(cfg.isConsistent());
}

TEST_F(TestCFG, First) {
    // Level: easy
    cfg << "<S> ::= <A><B>";
    cfg << "<A> ::= a<A>|";
    cfg << "<B> ::= b<B>|";
    ASSERT_EQ(set({"a", "b"}), cfg.first("<S>"));
    ASSERT_EQ(set({"a"}), cfg.first("<A>"));
    ASSERT_EQ(set({"b"}), cfg.first("<B>"));

    ASSERT_TRUE(cfg.nullable("<S>"));
    ASSERT_TRUE(cfg.nullable("<A>"));
    ASSERT_TRUE(cfg.nullable("<B>"));

    // Level: easy (but explores an edge case)
    cfg.clear();
    cfg.add("<S> ::= <S>a|");
    ASSERT_EQ(set({"a"}), cfg.first("<S>"));
    ASSERT_TRUE(cfg.nullable("<S>"));

    // Level: medium
    cfg.clear();
    cfg << "<E> ::= <T><E1>";
    cfg << "<E1> ::= +<T><E1>|";
    cfg << "<T> ::= <F><T1>";
    cfg << "<T1> ::= *<F><T1>|";
    cfg << "<F> ::= (<E>)|i";
    ASSERT_EQ(set({"(", "i"}), cfg.first("<E>"));
    ASSERT_EQ(set({"+"}), cfg.first("<E1>"));
    ASSERT_EQ(set({"(", "i"}), cfg.first("<T>"));
    ASSERT_EQ(set({"*"}), cfg.first("<T1>"));
    ASSERT_EQ(set({"(", "i"}), cfg.first("<F>"));

    ASSERT_FALSE(cfg.nullable("<E>"));
    ASSERT_TRUE(cfg.nullable("<E1>"));
    ASSERT_FALSE(cfg.nullable("<T>"));
    ASSERT_TRUE(cfg.nullable("<T1>"));
    ASSERT_FALSE(cfg.nullable("<F>"));
    ASSERT_EQ(set({"+", "(", "i"}), cfg.first("<E1><E>"));

    // Level: medium (with loop of recursions)
    cfg.clear();
    cfg << "<S> ::= <A>x|y";
    cfg << "<A> ::= <S>w|z";
    ASSERT_EQ(set({"y", "z"}), cfg.first("<S>"));
    ASSERT_EQ(set({"y", "z"}), cfg.first("<A>"));
    ASSERT_FALSE(cfg.nullable("<S>"));
    ASSERT_FALSE(cfg.nullable("<A>"));

    // Level: medium-hard
    cfg.clear();
    cfg << "<S> ::= <A><B><C><S>e|";
    cfg << "<A> ::= a<A>|";
    cfg << "<B> ::= b<B>|";
    cfg << "<C> ::= c<C>|";
    ASSERT_EQ(set({"a", "b", "c", "e"}), cfg.first("<S>"));
    ASSERT_EQ(set({"a"}), cfg.first("<A>"));
    ASSERT_EQ(set({"b"}), cfg.first("<B>"));
    ASSERT_EQ(set({"c"}), cfg.first("<C>"));
    ASSERT_TRUE(cfg.nullable("<S>"));
    ASSERT_TRUE(cfg.nullable("<A>"));
    ASSERT_TRUE(cfg.nullable("<B>"));
    ASSERT_TRUE(cfg.nullable("<C>"));

    // Level: hard
    cfg.clear();
    cfg << "<S> ::= <S>s|<B><C><D>";
    cfg << "<A> ::= <S><A>a|";
    cfg << "<B> ::= <C>c";
    cfg << "<C> ::= <B>b|<S>s|<A>";
    cfg << "<D> ::= <D>d|<D><B>|";
    ASSERT_EQ(set({"c"}), cfg.first("<S>"));
    ASSERT_EQ(set({"c"}), cfg.first("<A>"));
    ASSERT_EQ(set({"c"}), cfg.first("<B>"));
    ASSERT_EQ(set({"c"}), cfg.first("<C>"));
    ASSERT_EQ(set({"c", "d"}), cfg.first("<D>"));
    ASSERT_FALSE(cfg.nullable("<S>"));
    ASSERT_TRUE(cfg.nullable("<A>"));
    ASSERT_FALSE(cfg.nullable("<B>"));
    ASSERT_TRUE(cfg.nullable("<C>"));
    ASSERT_TRUE(cfg.nullable("<D>"));
}

TEST_F(TestCFG, Range) {
    cfg << "<S> ::= <S>a|<A><B>|c";
    cfg << "<A> ::= d<A>|<A>e|";
    cfg << "<B> ::= b";
    ASSERT_EQ(set({"<S>", "<A>", "<B>"}), cfg.range("<S>"));
    ASSERT_EQ(set({"<A>"}), cfg.range("<A>"));
    ASSERT_EQ(set(), cfg.range("<B>"));

    cfg.clear();
    cfg << "<S> ::= <S>s|<B><C><D>";
    cfg << "<A> ::= <S><A>a|";
    cfg << "<B> ::= <C>c";
    cfg << "<C> ::= <B>b|<S>s|<A>";
    cfg << "<D> ::= <D>d|<D><B>|";
    ASSERT_EQ(set({"<S>", "<A>", "<B>", "<C>"}), cfg.range("<S>"));
    ASSERT_EQ(set({"<S>", "<A>", "<B>", "<C>"}), cfg.range("<A>"));
    ASSERT_EQ(set({"<S>", "<A>", "<B>", "<C>"}), cfg.range("<B>"));
    ASSERT_EQ(set({"<S>", "<A>", "<B>", "<C>"}), cfg.range("<C>"));
    ASSERT_EQ(set({"<S>", "<A>", "<B>", "<C>", "<D>"}), cfg.range("<D>"));
}

TEST_F(TestCFG, Follow) {
    cfg << "<S> ::= <A><B><C>|e";
    cfg << "<A> ::= a<A>|";
    cfg << "<B> ::= b<B>|<A><C>d";
    cfg << "<C> ::= c<C>|";
    EXPECT_EQ(set(), cfg.follow("<S>"));
    EXPECT_EQ(set({"a", "b", "c", "d"}), cfg.follow("<A>"));
    EXPECT_EQ(set({"c"}), cfg.follow("<B>"));
    EXPECT_EQ(set({"d"}), cfg.follow("<C>"));
    EXPECT_TRUE(cfg.endable("<S>"));
    EXPECT_FALSE(cfg.endable("<A>"));
    EXPECT_TRUE(cfg.endable("<B>"));
    EXPECT_TRUE(cfg.endable("<C>"));

    cfg.clear();
    cfg << "<E> ::= <T><E1>";
    cfg << "<E1> ::= +<T><E1>|";
    cfg << "<T> ::= <F><T1>";
    cfg << "<T1> ::= *<F><T1>|";
    cfg << "<F> ::= (<E>)|i";
    EXPECT_EQ(set({")"}), cfg.follow("<E>"));
    EXPECT_EQ(set({")"}), cfg.follow("<E1>"));
    EXPECT_EQ(set({")", "+"}), cfg.follow("<T>"));
    EXPECT_EQ(set({")", "+"}), cfg.follow("<T1>"));
    EXPECT_EQ(set({")", "+", "*"}), cfg.follow("<F>"));
    EXPECT_TRUE(cfg.endable("<E>"));
    EXPECT_TRUE(cfg.endable("<E1>"));
    EXPECT_TRUE(cfg.endable("<T>"));
    EXPECT_TRUE(cfg.endable("<T1>"));
    EXPECT_TRUE(cfg.endable("<F>"));
}

TEST_F(TestCFG, Recursion) {
    cfg << "<S> ::= a<S>b|";
    EXPECT_FALSE(cfg.isRecursive());

    cfg.clear();
    cfg << "<S> ::= <S>a|";
    EXPECT_TRUE(cfg.isRecursive());

    cfg.clear();
    cfg << "<S> ::= <A><S><B>|";
    cfg << "<A> ::= a|";
    cfg << "<B> ::= <S>b|c";
    ASSERT_TRUE(cfg.isRecursive());
    EXPECT_EQ(CFG::DIRECT, cfg.recursionType("<S>"));
    EXPECT_EQ(CFG::NONE, cfg.recursionType("<A>"));
    EXPECT_EQ(CFG::INDIRECT, cfg.recursionType("<B>"));
}

TEST_F(TestCFG, Factorization) {
    cfg << "<S> ::= a<S>b|";
    EXPECT_TRUE(cfg.isFactored());

    cfg.clear();
    cfg << "<S> ::= <A><S><B>|";
    cfg << "<A> ::= a|";
    cfg << "<B> ::= <S>b|<C>";
    cfg << "<C> ::= c<C>|c<A>|";
    ASSERT_FALSE(cfg.isFactored());
    EXPECT_EQ(CFG::NONE, cfg.nonFactoringType("<S>"));
    EXPECT_EQ(CFG::NONE, cfg.nonFactoringType("<A>"));
    EXPECT_EQ(CFG::INDIRECT, cfg.nonFactoringType("<B>"));
    EXPECT_EQ(CFG::DIRECT, cfg.nonFactoringType("<C>"));
}

TEST_F(TestCFG, RecursionElimination) {
    cfg << "<S> ::= <S>a|b";
    CFG expected;
    expected << "<S> ::= b<S'>";
    expected << "<S'> ::= a<S'>|";
    EXPECT_EQ(expected, cfg.withoutRecursion());

    cfg.clear();
    cfg << "<S> ::= <S><S>a|b<A>";
    cfg << "<A> ::= b<B>c|<A>e|";
    cfg << "<B> ::= <B>a|<B>b|c|d|";
    expected.clear();
    expected << "<S> ::= b<A><S'>";
    expected << "<S'> ::= <S>a<S'>|";
    expected << "<A> ::= b<B>c<A'>|<A'>";
    expected << "<A'> ::= e<A'>|";
    expected << "<B> ::= c<B'>|d<B'>|<B'>";
    expected << "<B'> ::= a<B'>|b<B'>|";
    EXPECT_EQ(expected, cfg.withoutRecursion());

    // cfg.clear();
    // cfg << "<S> ::= <A>a|a";
    // cfg << "<A> ::= <B>b|b";
    // cfg << "<B> ::= <S>c|c";
    // expected.clear();
    // expected << "<S> ::= <A>a|a";
    // expected << "<A> ::= <B>b|b";
    // expected << "<B> ::= bac<B'>|ac<B'>|c<B'>";
    // expected << "<B'> ::= bac<B'>|";
    // EXPECT_EQ(expected, cfg.withoutRecursion());
}

TEST_F(TestCFG, RepresentationExchange) {
    auto test = CFG(DidacticNotation());
    ASSERT_NO_THROW(test << "S -> a A b | ");
    ASSERT_NO_THROW(test << "A -> a A | b A | ");
    EXPECT_EQ(set({"a", "b"}), test.getTerminals());
    EXPECT_EQ(set({"S", "A"}), test.getNonTerminals());

    // test.clear();
    // test << "S -> a A b | ";

    // test << "S -> S s | B C D";
    // test << "A -> S A a | ";
    // test << "B -> C c";
    // test << "C -> B b | S s | A";
    // test << "D -> D d | D B | ";
    // ASSERT_EQ(set({"c"}), test.first("S"));
    // ASSERT_EQ(set({"c"}), test.first("A"));
    // ASSERT_EQ(set({"c"}), test.first("B"));
    // ASSERT_EQ(set({"c"}), test.first("C"));
    // ASSERT_EQ(set({"c", "d"}), test.first("D"));
    // ASSERT_FALSE(test.nullable("S"));
    // ASSERT_TRUE(test.nullable("A"));
    // ASSERT_FALSE(test.nullable("B"));
    // ASSERT_TRUE(test.nullable("C"));
    // ASSERT_TRUE(test.nullable("D"));
}

// TEST_F(TestCFG, FactorizationElimination) {
//     cfg << "<S> ::= a<S>b|ac";
//     CFG expected;
//     expected << "<S> ::= a<S'>";
//     expected << "<S'> ::= <S>b|c";
//     EXPECT_EQ(expected, cfg.factored());

//     cfg << "<S> ::= a<S>a|aa";
//     EXPECT_ANY_THROW(cfg.factored(5));
// }

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
