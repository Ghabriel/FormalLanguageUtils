/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <gtest/gtest.h>
#include "Regex.hpp"

class TestRegex : public ::testing::Test {};

TEST_F(TestRegex, BasicMatching1) {
    Regex regex("abc");
    ASSERT_FALSE(regex.matches("ab"));
    ASSERT_FALSE(regex.matches("xyz"));
    ASSERT_TRUE(regex.matches("abc"));
    ASSERT_FALSE(regex.matches("abcd"));

    regex = Regex("ab*c");
    ASSERT_FALSE(regex.matches("ab"));
    ASSERT_FALSE(regex.matches("xyz"));
    ASSERT_TRUE(regex.matches("abc"));
    ASSERT_FALSE(regex.matches("abcd"));
    ASSERT_TRUE(regex.matches("ac"));
    ASSERT_TRUE(regex.matches("abbc"));
    ASSERT_TRUE(regex.matches("abbbbbbbc"));
    ASSERT_FALSE(regex.matches("abbbbbbbcc"));

    regex = Regex("ab+c");
    ASSERT_FALSE(regex.matches("ab"));
    ASSERT_FALSE(regex.matches("xyz"));
    ASSERT_TRUE(regex.matches("abc"));
    ASSERT_FALSE(regex.matches("abcd"));
    ASSERT_FALSE(regex.matches("ac"));
    ASSERT_TRUE(regex.matches("abbc"));
    ASSERT_TRUE(regex.matches("abbbbbbbc"));
    ASSERT_FALSE(regex.matches("abbbbbbbcc"));
}

TEST_F(TestRegex, BasicMatching2) {
    Regex regex("ab+c|ac*b");
    ASSERT_TRUE(regex.matches("abbbbc"));
    ASSERT_TRUE(regex.matches("accccccb"));
    ASSERT_TRUE(regex.matches("ab"));
    ASSERT_FALSE(regex.matches("abbccb"));
    ASSERT_FALSE(regex.matches("abbccb"));

    regex = Regex("(ba|a(ba)*a)*(ab)*");
    ASSERT_TRUE(regex.matches("bababaabababaaba"));
    ASSERT_TRUE(regex.matches("ababab"));
    ASSERT_FALSE(regex.matches("abbaba"));
    ASSERT_FALSE(regex.matches("ababa"));
    ASSERT_TRUE(regex.matches("aaaaaaaaaaaa"));
    ASSERT_FALSE(regex.matches("aaaaaaaaaabb"));

    regex = Regex("0?(10)*1?");
    ASSERT_TRUE(regex.matches(""));
    ASSERT_TRUE(regex.matches("0"));
    ASSERT_TRUE(regex.matches("1"));
    ASSERT_TRUE(regex.matches("01010101010101"));
    ASSERT_TRUE(regex.matches("01010101010"));
    ASSERT_TRUE(regex.matches("101010101"));
    ASSERT_TRUE(regex.matches("1010101010"));
    ASSERT_FALSE(regex.matches("0110101010"));
    ASSERT_FALSE(regex.matches("10010101010101"));
    ASSERT_FALSE(regex.matches("00110011"));
}

TEST_F(TestRegex, ProgressiveScan) {
    Regex regex("ab+c?");
    regex.read('a');
    ASSERT_FALSE(regex.matches());
    ASSERT_FALSE(regex.aborted());
    regex.read('b');
    ASSERT_TRUE(regex.matches());
    ASSERT_FALSE(regex.aborted());
    regex.read('c');
    ASSERT_TRUE(regex.matches());
    ASSERT_FALSE(regex.aborted());
    regex.read('d');
    ASSERT_FALSE(regex.matches());
    ASSERT_TRUE(regex.aborted());

    regex.reset();
    regex.read('a');
    ASSERT_FALSE(regex.matches());
    ASSERT_FALSE(regex.aborted());
    regex.read('b');
    ASSERT_TRUE(regex.matches());
    ASSERT_FALSE(regex.aborted());
    regex.read('b');
    ASSERT_TRUE(regex.matches());
    ASSERT_FALSE(regex.aborted());
    regex.read('c');
    ASSERT_TRUE(regex.matches());
    ASSERT_FALSE(regex.aborted());
    regex.read('c');
    ASSERT_FALSE(regex.matches());
    ASSERT_TRUE(regex.aborted());
}

TEST_F(TestRegex, Wildcard) {
    Regex regex(".");
    ASSERT_TRUE(regex.matches("."));
    ASSERT_TRUE(regex.matches("a"));
    ASSERT_TRUE(regex.matches("b"));
    ASSERT_TRUE(regex.matches("z"));
    ASSERT_TRUE(regex.matches("5"));
    ASSERT_TRUE(regex.matches("@"));
    ASSERT_FALSE(regex.matches(""));
    ASSERT_FALSE(regex.matches(".."));
    ASSERT_FALSE(regex.matches("az"));

    regex = Regex("a+.*z?");
    ASSERT_TRUE(regex.matches("a"));
    ASSERT_TRUE(regex.matches("aaaskm@mk94mkz"));
    ASSERT_TRUE(regex.matches("aaak2l¬*l$k"));
    ASSERT_FALSE(regex.matches("bz"));
    ASSERT_FALSE(regex.matches("z"));

    regex = Regex(".*@.+@");
    ASSERT_TRUE(regex.matches("@a@"));
    ASSERT_TRUE(regex.matches("abc@@@"));
    ASSERT_TRUE(regex.matches("@xyz@"));
    ASSERT_TRUE(regex.matches("abc@xyz@"));
    ASSERT_TRUE(regex.matches("@@@"));
    ASSERT_TRUE(regex.matches("@@@@@@@@@"));
    ASSERT_FALSE(regex.matches("@a"));
    ASSERT_FALSE(regex.matches("@@"));

    regex = Regex(".*@.*|.*$.*|Z");
    ASSERT_TRUE(regex.matches("a@b"));
    ASSERT_TRUE(regex.matches("a$b"));
    ASSERT_TRUE(regex.matches("a@b$c"));
    ASSERT_TRUE(regex.matches("a$b@c"));
    ASSERT_TRUE(regex.matches("Z"));
    ASSERT_TRUE(regex.matches("Z@a"));
    ASSERT_FALSE(regex.matches("ZZ"));
    ASSERT_FALSE(regex.matches("abc"));
    ASSERT_FALSE(regex.matches("Zabc"));
}

TEST_F(TestRegex, CharClasses) {
    Regex regex("[0-9]+");
    ASSERT_TRUE(regex.matches("0"));
    ASSERT_TRUE(regex.matches("5"));
    ASSERT_TRUE(regex.matches("10239023"));
    ASSERT_FALSE(regex.matches("381933d12938"));
    ASSERT_FALSE(regex.matches("z"));
    ASSERT_FALSE(regex.matches(""));
    ASSERT_FALSE(regex.matches("@"));

    regex = Regex("[A-Za-z_][A-Za-z0-9_]* = [0-9]+");
    ASSERT_TRUE(regex.matches("three = 3"));
    ASSERT_TRUE(regex.matches("_valid = 77"));
    ASSERT_TRUE(regex.matches("_a10 = 2"));
    ASSERT_TRUE(regex.matches("_ = 3"));
    ASSERT_FALSE(regex.matches("3ab = 9"));
    ASSERT_FALSE(regex.matches("num$ = 0"));
    ASSERT_FALSE(regex.matches("$x = 4"));
    ASSERT_FALSE(regex.matches("abc = def"));
}

TEST_F(TestRegex, EscapeSequences) {
    Regex regex("\\(.*\\)");
    ASSERT_TRUE(regex.matches("()"));
    ASSERT_TRUE(regex.matches("(abc)"));
    ASSERT_TRUE(regex.matches("())))()((()()()()()"));
    ASSERT_FALSE(regex.matches("("));
    ASSERT_FALSE(regex.matches(")"));
    ASSERT_FALSE(regex.matches(")("));
    ASSERT_FALSE(regex.matches("(abcka$sd#mk@md"));
    ASSERT_FALSE(regex.matches("(abc)def"));

    regex = Regex("[0-9]+\\.?[0-9]*|\\.[0-9]+");
    ASSERT_TRUE(regex.matches("29302930"));
    ASSERT_TRUE(regex.matches("10230.23123"));
    ASSERT_TRUE(regex.matches(".8245227"));
    ASSERT_TRUE(regex.matches("3.1415926"));
    ASSERT_TRUE(regex.matches("965."));
    ASSERT_TRUE(regex.matches(".3"));
    ASSERT_FALSE(regex.matches(""));
    ASSERT_FALSE(regex.matches("123.456.789"));
    ASSERT_FALSE(regex.matches("."));
    ASSERT_FALSE(regex.matches("1234S6789"));
    ASSERT_FALSE(regex.matches("@!*#&!@(*&"));

    regex = Regex("[0-9]+\n[A-Z]+");
    ASSERT_TRUE(regex.matches("123\nABC"));
    ASSERT_TRUE(regex.matches("1\nK"));
    ASSERT_FALSE(regex.matches("K\n1"));
    ASSERT_FALSE(regex.matches("2 S"));
    ASSERT_FALSE(regex.matches("9"));
    ASSERT_FALSE(regex.matches("Z"));
}

TEST_F(TestRegex, CountedRepetition) {
    Regex regex("a{3}b{4}");
    ASSERT_TRUE(regex.matches("aaabbbb"));
    ASSERT_FALSE(regex.matches(""));
    ASSERT_FALSE(regex.matches("ab"));
    ASSERT_FALSE(regex.matches("aaabbb"));
    ASSERT_FALSE(regex.matches("aabbbb"));

    regex = Regex(".{3,8}");
    ASSERT_TRUE(regex.matches("@#$"));
    ASSERT_TRUE(regex.matches("1234"));
    ASSERT_TRUE(regex.matches("pimpl"));
    ASSERT_TRUE(regex.matches("abcd5678"));
    ASSERT_FALSE(regex.matches(""));
    ASSERT_FALSE(regex.matches("p*"));
    ASSERT_FALSE(regex.matches("Y"));
    ASSERT_FALSE(regex.matches("123456789"));

    regex = Regex("a{2,}");
    ASSERT_TRUE(regex.matches("aa"));
    ASSERT_TRUE(regex.matches("aaaaaaaaaaaaaaa"));
    ASSERT_FALSE(regex.matches(""));
    ASSERT_FALSE(regex.matches("a"));
    ASSERT_FALSE(regex.matches("aaaaaaaaaaaaaaab"));
    ASSERT_FALSE(regex.matches("h"));
}

TEST_F(TestRegex, FinalTest) {
    Regex regex(
        "[A-Za-z0-9_ ]+ \\(([0-2][0-9]|3[0-1])\\.(0[0-9]|1[0-2])\\.[0-9]{0,4}\\)"
    );
    ASSERT_TRUE(regex.matches("Albert Einstein (14.03.1879)"));
    ASSERT_TRUE(regex.matches("Isaac Newton (04.01.1643)"));
    ASSERT_TRUE(regex.matches("Marie Curie (07.11.1867)"));
    ASSERT_TRUE(regex.matches("Nikola Tesla (10.07.1856)"));
    ASSERT_TRUE(regex.matches("Stephen Hawking (08.01.1942)"));
    ASSERT_TRUE(regex.matches("Today (30.07.2016)"));
    ASSERT_TRUE(regex.matches("1234 (31.12.9999)"));
    ASSERT_FALSE(regex.matches("wtf (32.01.2016)"));
    ASSERT_FALSE(regex.matches("wtf (20.13.2016)"));
    ASSERT_FALSE(regex.matches("wtf (01.01.10000)"));
    ASSERT_FALSE(regex.matches("wtf (01.01.2016"));
    ASSERT_FALSE(regex.matches("(01.01.2016)"));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
