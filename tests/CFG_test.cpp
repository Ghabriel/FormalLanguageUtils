/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <gtest/gtest.h>
#include "CFG.hpp"

using set = std::unordered_set<std::string>;

class TestCFG : public ::testing::Test {
protected:
    CFG cfg;
};

TEST_F(TestCFG, SymbolLists) {
    ASSERT_NO_THROW(cfg.add("S", {"a", "<A>", "b"}, {"<&>"}));
    ASSERT_NO_THROW(cfg.add("A", {"a", "<A>"}, {"b", "<A>"}, {"<&>"}));
    set expectedTerminals = {"a", "b"};
    set expectedNonTerminals = {"S", "A"};
    EXPECT_EQ(cfg.getTerminals(), expectedTerminals);
    EXPECT_EQ(cfg.getNonTerminals(), expectedNonTerminals);
}

TEST_F(TestCFG, Consistency) {
    EXPECT_TRUE(cfg.isConsistent());
    cfg.add("S", {"a", "<A>", "b"}, {"<&>"});
    EXPECT_FALSE(cfg.isConsistent());
    cfg.add("A", {"a", "<A>"}, {"b", "<A>"}, {"<&>"});
    EXPECT_TRUE(cfg.isConsistent());
}

TEST_F(TestCFG, Range) {
    cfg.add("<S> ::= <S>a | <A><B> | c");
    cfg.add("<A> ::= d<A> | <A>e | <&>");
    cfg.add("<B> ::= b");
    set expRangeS = {"S", "A", "B"};
    set expRangeA = {"A"};
    set expRangeB;

    cfg.updateRangeTable();
    EXPECT_EQ(expRangeS, cfg.range("S"));
    EXPECT_EQ(expRangeA, cfg.range("A"));
    EXPECT_EQ(expRangeB, cfg.range("B"));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
