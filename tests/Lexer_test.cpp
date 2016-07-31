/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <gtest/gtest.h>
#include "Lexer.hpp"

class TestLexer : public ::testing::Test {
protected:
    Lexer lexer;
};

// TEST_F(TestLexer, Test1) {
//     lexer.add("NUMBER", "[0-9]+\\.?[0-9]*|\\.[0-9]+");
//     lexer.add("ASSIGNMENT", "=");
//     lexer.add("IDENTIFIER", "[A-Za-z_]+[A-Za-z0-9_]*");
//     lexer.read("aaabbab");
//     ASSERT_TRUE(lexer.accepts());

//     std::queue<std::string> tokens;
//     ASSERT_NO_THROW(tokens = grammar.tokens());
//     std::queue<std::string> expected;
//     expected.push("");
// }

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
