/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <gtest/gtest.h>
#include "CFG.hpp"

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

// TEST_F(TestCFG, Range) {
//     cfg << "<S> ::= <S>a | <A><B> | c";
//     cfg << "<A> ::= d<A> | <A>e |";
//     cfg << "<B> ::= b";

//     EXPECT_EQ(set({"S", "A", "B"}), cfg.range("<S>"));
//     EXPECT_EQ(set({"A"}), cfg.range("<A>"));
//     EXPECT_EQ(set(), cfg.range("<B>"));
// }

TEST_F(TestCFG, First) {
    ASSERT_NO_THROW(cfg << "<E> ::= <T><E1>");
    ASSERT_NO_THROW(cfg << "<E1> ::= +<T><E1>|");
    ASSERT_NO_THROW(cfg << "<T> ::= <F><T1>");
    ASSERT_NO_THROW(cfg << "<T1> ::= *<F><T1>|");
    ASSERT_NO_THROW(cfg << "<F> ::= (<E>)|id");

    ASSERT_EQ(set({"(", "id"}), cfg.first("<E>"));
    EXPECT_EQ(set({"+"}), cfg.first("<E1>"));
    EXPECT_EQ(set({"(", "id"}), cfg.first("<T>"));
    EXPECT_EQ(set({"*"}), cfg.first("<T1>"));
    EXPECT_EQ(set({"(", "id"}), cfg.first("<F>"));

    EXPECT_FALSE(cfg.nullable("<E>"));
    EXPECT_TRUE(cfg.nullable("<E1>"));
    EXPECT_FALSE(cfg.nullable("<T>"));
    EXPECT_TRUE(cfg.nullable("<T1>"));
    EXPECT_FALSE(cfg.nullable("<F>"));
    EXPECT_EQ(set({"+", "(", "id"}), cfg.first("<E1><E>"));

    cfg.clear();
    cfg.add("<S> ::= <S>a|");
    EXPECT_EQ(set({"a"}), cfg.first("<S>"));
    EXPECT_TRUE(cfg.nullable("<S>"));

    cfg.clear();
    cfg << "<S> ::= <A><B><C><S>e|";
    cfg << "<A> ::= a<A>|";
    cfg << "<B> ::= b<B>|";
    cfg << "<C> ::= c<C>|";
    EXPECT_EQ(set({"a", "b", "c", "e"}), cfg.first("<S>"));
    EXPECT_EQ(set({"a"}), cfg.first("<A>"));
    EXPECT_EQ(set({"a"}), cfg.first("<B>"));
    EXPECT_EQ(set({"a"}), cfg.first("<C>"));
    EXPECT_TRUE(cfg.nullable("<S>"));
    EXPECT_TRUE(cfg.nullable("<A>"));
    EXPECT_TRUE(cfg.nullable("<B>"));
    EXPECT_TRUE(cfg.nullable("<C>"));
}

// TEST_F(TestCFG, Follow) {
//     cfg << "<S> ::= <A><B><C>|e";
//     cfg << "<A> ::= a<A>|";
//     cfg << "<B> ::= b<B>|<A><C>d";
//     cfg << "<C> ::= c<C>|";
//     EXPECT_EQ(set(), cfg.follow("<S>"));
//     EXPECT_EQ(set({"a", "b", "c", "d"}), cfg.follow("<A>"));
//     EXPECT_EQ(set({"c"}), cfg.follow("<B>"));
//     EXPECT_EQ(set({"d"}), cfg.follow("<C>"));
//     EXPECT_TRUE(cfg.endable("<S>"));
//     EXPECT_FALSE(cfg.endable("<A>"));
//     EXPECT_TRUE(cfg.endable("<B>"));
//     EXPECT_TRUE(cfg.endable("<C>"));

//     cfg << "<E> ::= <T><E1>";
//     cfg << "<E1> ::= +<T><E1>|";
//     cfg << "<T> ::= <F><T1>";
//     cfg << "<T1> ::= *<F><T1>|";
//     cfg << "<F> ::= (<E>)|id";
//     EXPECT_EQ(set({")"}), cfg.follow("<E>"));
//     EXPECT_EQ(set({")"}), cfg.follow("<E1>"));
//     EXPECT_EQ(set({")", "+"}), cfg.follow("<T>"));
//     EXPECT_EQ(set({")", "+"}), cfg.follow("<T1>"));
//     EXPECT_EQ(set({")", "+", "*"}), cfg.follow("<F>"));
//     EXPECT_TRUE(cfg.endable("<E>"));
//     EXPECT_TRUE(cfg.endable("<E1>"));
//     EXPECT_TRUE(cfg.endable("<T>"));
//     EXPECT_TRUE(cfg.endable("<T1>"));
//     EXPECT_TRUE(cfg.endable("<F>"));
// }

// TEST_F(TestCFG, Recursion) {
//     cfg << "<S> ::= a<S>b|";
//     EXPECT_FALSE(cfg.isRecursive());

//     cfg.clear();
//     cfg << "<S> ::= <S>a|";
//     EXPECT_TRUE(cfg.isRecursive());

//     cfg.clear();
//     cfg << "<S> ::= <A><S><B>|";
//     cfg << "<A> ::= a|";
//     cfg << "<B> ::= <S>b|c";
//     ASSERT_TRUE(cfg.isRecursive());
//     EXPECT_EQ(CFG::DIRECT, cfg.recursionType("<S>"));
//     EXPECT_EQ(CFG::NONE, cfg.recursionType("<A>"));
//     EXPECT_EQ(CFG::INDIRECT, cfg.recursionType("<B>"));
// }

// TEST_F(TestCFG, Factorization) {
//     cfg << "<S> ::= a<S>b|";
//     EXPECT_TRUE(cfg.isFactored());

//     cfg.clear();
//     cfg << "<S> ::= <A><S><B>|";
//     cfg << "<A> ::= a|";
//     cfg << "<B> ::= <S>b|<C>";
//     cfg << "<C> ::= c<C>|c<A>|";
//     ASSERT_FALSE(cfg.isFactored());
//     EXPECT_EQ(CFG::NONE, cfg.nonFactoringType("<S>"));
//     EXPECT_EQ(CFG::NONE, cfg.nonFactoringType("<A>"));
//     EXPECT_EQ(CFG::INDIRECT, cfg.nonFactoringType("<B>"));
//     EXPECT_EQ(CFG::DIRECT, cfg.nonFactoringType("<C>"));
// }

// TEST_F(TestCFG, RecursionElimination) {
//     cfg << "<S> ::= <S>a|b";
//     CFG expected;
//     expected << "<S> ::= b<S'>";
//     expected << "<S'> ::= a<S'>|";
//     EXPECT_EQ(expected, cfg.withoutRecursion());

//     cfg.clear();
//     cfg << "<S> ::= <A>a|a";
//     cfg << "<A> ::= <B>b|b";
//     cfg << "<B> ::= <S>c|c";
//     expected.clear();
//     expected << "<S> ::= <A>a|a";
//     expected << "<A> ::= <B>b|b";
//     expected << "<B> ::= bac<B'>|ac<B'>|c<B'>";
//     expected << "<B'> ::= bac<B'>|";
//     EXPECT_EQ(expected, cfg.withoutRecursion());
// }

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
