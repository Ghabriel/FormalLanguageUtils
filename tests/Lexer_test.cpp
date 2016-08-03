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
    lexer.addDelimiters(" ");

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

    EXPECT_NO_THROW(lexer.read("192.168.0.1"));
    EXPECT_FALSE(lexer.accepts());
}

TEST_F(TestLexer, Test2) {
    lexer.addToken("TYPE", "int|float|double|char|unsigned|string");
    lexer.addToken("EQUAL", "=");
    lexer.addToken("WHILE", "while");
    lexer.addToken("(", "\\(");
    lexer.addToken(")", "\\)");
    lexer.addToken("{", "\\{");
    lexer.addToken("}", "\\}");
    lexer.addToken(";", ";");
    lexer.addToken("ARITHMETIC_OPERATOR", "\\+|-|\\*|/|%");
    lexer.addToken("COMPARATOR", "<|>|<=|>=|==");
    lexer.addToken("BINARY_OPERATORS", "^|&|\\|");
    lexer.addToken("NUMBER", "[0-9]+\\.?[0-9]*|\\.[0-9]+");
    lexer.addToken("IDENTIFIER", "[A-Za-z_][A-Za-z0-9_]*");
    lexer.ignore(' ');
    lexer.ignore('\n');
    lexer.addDelimiters("[^A-Za-z0-9_.]");

    std::vector<Token> tokens;
    ASSERT_NO_THROW(tokens = lexer.read("int i = 0;\nwhile ( i < size ) {\n\n}"));

    std::vector<Token> expected;
    expected.push_back({"TYPE", "int"});
    expected.push_back({"IDENTIFIER", "i"});
    expected.push_back({"EQUAL", "="});
    expected.push_back({"NUMBER", "0"});
    expected.push_back({";", ";"});
    expected.push_back({"WHILE", "while"});
    expected.push_back({"(", "("});
    expected.push_back({"IDENTIFIER", "i"});
    expected.push_back({"COMPARATOR", "<"});
    expected.push_back({"IDENTIFIER", "size"});
    expected.push_back({")", ")"});
    expected.push_back({"{", "{"});
    expected.push_back({"}", "}"});
    EXPECT_EQ(expected, tokens);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
