/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <gtest/gtest.h>
#include "Lexer.hpp"

class TestLexer : public ::testing::Test {
protected:
    Lexer lexer;
};

TEST_F(TestLexer, Test1) {
    lexer.addToken("T_NUMBER", "[0-9]+\\.?[0-9]*|\\.[0-9]+");
    lexer.addToken("T_PLUS", "\\+");
    lexer.addToken("T_TIMES", "\\*");
    lexer.ignore(' ');

    std::vector<Token> tokens;
    ASSERT_NO_THROW(tokens = lexer.read("22 3.14 + * 7 + 9"));
    ASSERT_TRUE(lexer.accepts());

    std::vector<Token> expected;
    expected.push_back({"T_NUMBER", "22"});
    expected.push_back({"T_NUMBER", "3.14"});
    expected.push_back({"T_PLUS", "+"});
    expected.push_back({"T_TIMES", "*"});
    expected.push_back({"T_NUMBER", "7"});
    expected.push_back({"T_PLUS", "+"});
    expected.push_back({"T_NUMBER", "9"});
    EXPECT_EQ(expected, tokens);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
