/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <gtest/gtest.h>
#include "CFG.hpp"

using set = std::unordered_set<std::string>;

class TestCFG : public ::testing::Test {
protected:
    CFG cfg;
};

TEST_F(TestCFG, SymbolLists) {
    ASSERT_NO_THROW(cfg.add("S", "a<A>b", "<&>"));
    ASSERT_NO_THROW(cfg.add("A", "a<A>", "b<A>", "<&>"));
    EXPECT_EQ(set({"a", "b"}), cfg.getTerminals());
    EXPECT_EQ(set({"S", "A"}), cfg.getNonTerminals());
}

TEST_F(TestCFG, Consistency) {
    EXPECT_TRUE(cfg.isConsistent());
    cfg.add("S", "a<A>b", "<&>");
    EXPECT_FALSE(cfg.isConsistent());
    cfg.add("A", "a<A>", "b<A>", "<&>");
    EXPECT_TRUE(cfg.isConsistent());
}

TEST_F(TestCFG, Range) {
    cfg.add("<S> ::= <S>a | <A><B> | c");
    cfg.add("<A> ::= d<A> | <A>e | <&>");
    cfg.add("<B> ::= b");

    EXPECT_EQ(set({"S", "A", "B"}), cfg.range("<S>"));
    EXPECT_EQ(set({"A"}), cfg.range("<A>"));
    EXPECT_EQ(set(), cfg.range("<B>"));
}

TEST_F(TestCFG, First) {
    cfg.add("<E> ::= <T><E1>");
    cfg.add("<E1> ::= +<T><E1> | <&>");
    cfg.add("<T> ::= <F><T1>");
    cfg.add("<T1> ::= *<F><T1> | <&>");
    cfg.add("<F> ::= (<E>) | id");

    EXPECT_EQ(set({"(", "id"}), cfg.first("<E>"));
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
    cfg.add("<S> ::= <S>a | <&>");
    EXPECT_EQ(set({"a"}), cfg.first("<S>"));
    EXPECT_TRUE(cfg.nullable("<S>"));

    cfg.clear();
    cfg.add("<S> ::= <A><B><C><S>e | <&>");
    cfg.add("<A> ::= a<A> | <&>");
    cfg.add("<B> ::= b<B> | <&>");
    cfg.add("<C> ::= c<C> | <&>");
    EXPECT_EQ(set({"a", "b", "c", "e"}). cfg.first("<S>"));
    EXPECT_EQ(set({"a"}). cfg.first("<A>"));
    EXPECT_EQ(set({"a"}). cfg.first("<B>"));
    EXPECT_EQ(set({"a"}). cfg.first("<C>"));
    EXPECT_TRUE(cfg.nullable("<S>"));
    EXPECT_TRUE(cfg.nullable("<A>"));
    EXPECT_TRUE(cfg.nullable("<B>"));
    EXPECT_TRUE(cfg.nullable("<C>"));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
