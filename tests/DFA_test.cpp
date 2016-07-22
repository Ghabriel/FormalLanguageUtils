#include <gtest/gtest.h>
#include "DFA.hpp"

class TestDFA : public ::testing::Test {
protected:
    DFA instance;
};

TEST_F(TestDFA, InitialState) {
    instance << "q0";
    EXPECT_EQ("q0", instance.initialState());
}

TEST_F(TestDFA, InitialStateSet) {
    instance << "q0";
    instance << "q1";
    instance.initialState("q1");
    instance.reset();
    EXPECT_EQ("q1", instance.initialState());
}

TEST_F(TestDFA, Transitions) {
    instance << "q0";
    instance << "q1";
    instance << "q2";
    instance.addTransition("q0", "q1", 'a');
    instance.addTransition("q1", "q2", 'b');
    EXPECT_EQ(instance.initialState(), instance.state());
    instance.read("ab");
    EXPECT_EQ("q2", instance.state().getName());
}

TEST_F(TestDFA, Reset) {
    instance << "q0";
    instance << "q1";
    instance.addTransition("q0", "q1", 'a');
    instance.read("a");
    instance.reset();
    EXPECT_EQ(instance.initialState(), instance.state());
}

TEST_F(TestDFA, Acceptance) {
    instance << "q0";
    instance << "q1";
    instance << "q2";
    instance.addTransition("q0", "q1", 'a');
    instance.addTransition("q1", "q2", 'b');
    instance.addTransition("q2", "q1", 'b');
    instance.accept("q2");

    instance.read("ab");
    EXPECT_TRUE(instance.accepts());
    instance.reset();
    instance.read("abb");
    EXPECT_FALSE(instance.accepts());
}

TEST_F(TestDFA, ErrorState) {
    EXPECT_TRUE(instance.error());
    instance << "q0";
    instance.addTransition("q0", "q0", 'c');
    EXPECT_FALSE(instance.error());
    instance.read("a");
    EXPECT_TRUE(instance.error());
    instance.read("b");
    EXPECT_TRUE(instance.error());
    instance.reset();
    EXPECT_FALSE(instance.error());
    instance.read("c");
    EXPECT_FALSE(instance.error());
}

TEST_F(TestDFA, Stress) {
    auto fn = [](unsigned i) -> std::string {
        return std::to_string(i);
    };

    unsigned limit = 1e5;
    instance.reserve(limit);
    std::string input;
    for (unsigned i = 0; i < limit; i++) {
        instance << ("q" + fn(i));
        input += "a";
    }

    for (unsigned i = 0; i < limit; i++) {
        instance.addTransition("q" + fn(i), "q" + fn((i+1) % limit), 'a');
    }

    instance.read(input);
    EXPECT_EQ("q0", instance.state().getName());

    input += "aaaa";
    instance.read(input);
    EXPECT_EQ("q4", instance.state().getName());
}

TEST_F(TestDFA, Size) {
    EXPECT_EQ(0, instance.size());
    instance << "q0";
    EXPECT_EQ(1, instance.size());
    instance << "q1";
    EXPECT_EQ(2, instance.size());
    instance << "q2";
    EXPECT_EQ(3, instance.size());
    instance << "q3";
    EXPECT_EQ(4, instance.size());
}

TEST_F(TestDFA, Alphabet) {
    instance << "q0";
    instance << "q1";
    instance << "q2";
    instance.addTransition("q0", "q0", 'a');
    instance.addTransition("q0", "q0", 'c');
    instance.addTransition("q0", "q0", 'z');
    instance.addTransition("q0", "q1", 'b');
    instance.addTransition("q0", "q1", 'f');
    instance.addTransition("q1", "q1", 'd');
    instance.addTransition("q2", "q2", 'g');
    std::unordered_set<char> expected = {
        'a', 'c', 'z', 'b', 'f', 'd', 'g'
    };
    EXPECT_EQ(expected, instance.alphabet());
}

TEST_F(TestDFA, AcceptingStates) {
    instance << "q0";
    instance << "q1";
    instance << "q2";
    instance << "q3";
    instance << "q4";
    instance.accept("q1", "q2", "q4");
    std::unordered_set<State> expected = {"q1", "q2", "q4"};
    EXPECT_EQ(3, instance.finalStates().size());
    EXPECT_EQ(expected, instance.finalStates());
}

TEST_F(TestDFA, DeadStateRemoval) {
    instance << "q0";
    instance << "q1";
    instance << "q2";
    instance << "q3";
    instance.addTransition("q0", "q1", 'a');
    instance.addTransition("q1", "q2", 'b');
    instance.addTransition("q2", "q2", 'c');
    instance.addTransition("q3", "q3", 'd');
    EXPECT_EQ(4, instance.size());
    DFA other = instance.withoutDeadStates();
    EXPECT_EQ(0, other.size());

    instance.accept("q1");
    other = instance.withoutDeadStates();
    EXPECT_EQ(2, other.size());

    instance.accept("q3");
    other = instance.withoutDeadStates();
    EXPECT_EQ(3, other.size());

    DFA empty;
    ASSERT_NO_THROW(empty.withoutDeadStates());
    EXPECT_EQ(0, empty.withoutDeadStates().size());
}

TEST_F(TestDFA, UnreachableStateRemoval) {
    instance << "q0";
    instance << "q1";
    instance << "q2";
    instance << "q3";
    instance.addTransition("q0", "q1", 'a');
    instance.addTransition("q1", "q2", 'b');
    instance.addTransition("q2", "q2", 'c');
    instance.addTransition("q3", "q3", 'd');
    EXPECT_EQ(4, instance.size());

    DFA other = instance.withoutUnreachableStates();
    EXPECT_EQ(3, other.size());

    DFA empty;
    ASSERT_NO_THROW(empty.withoutUnreachableStates());
    EXPECT_EQ(0, empty.withoutUnreachableStates().size());
}

TEST_F(TestDFA, Minimization) {
    instance << "q0";
    instance << "q1";
    instance << "q2";
    instance << "q3";
    instance.accept("q1");
    instance.addTransition("q0", "q1", 'a');
    instance.addTransition("q1", "q2", 'b');
    instance.addTransition("q2", "q2", 'c');
    instance.addTransition("q3", "q3", 'd');
    EXPECT_EQ(4, instance.size());
    
    DFA other = instance.minimized();
    EXPECT_EQ(2, other.size());
}

// TEST_F(TestDFA, Complement) {
//     instance << "q0";
//     instance << "q1";
//     instance << "q2";
//     instance.addTransition("q0", "q1", 'a');
//     instance.addTransition("q1", "q2", 'a');
//     instance.addTransition("q2", "q0", 'a');
//     instance.accept("q0");

//     DFA complement = !instance;

//     EXPECT_EQ(3, complement.size());

//     EXPECT_TRUE(instance.accepts());
//     EXPECT_FALSE(instance.accepts());

//     instance.read("a");
//     EXPECT_FALSE(instance.accepts());
//     EXPECT_TRUE(instance.accepts());

//     instance.read("a");
//     EXPECT_FALSE(instance.accepts());
//     EXPECT_TRUE(instance.accepts());

//     instance.read("a");
//     EXPECT_TRUE(instance.accepts());
//     EXPECT_FALSE(instance.accepts());
// }

// TEST_F(TestDFA, Intersection) {
//     instance << "q0";
//     instance << "q1";
//     instance << "q2";
//     instance.addTransition("q0", "q1", 'a');
//     instance.addTransition("q1", "q2", 'a');
//     instance.addTransition("q2", "q0", 'a');
//     instance.accept("q0");

//     DFA second;
//     second << "q0";
//     second << "q1";
//     second.addTransition("q0", "q1", 'a');
//     second.addTransition("q1", "q0", 'a');
//     second.accept("q0");

//     DFA intersection = instance && second;
//     EXPECT_EQ(6, intersection.size());
//     EXPECT_TRUE(intersection.accepts());
//     intersection.read("a");
//     EXPECT_FALSE(intersection.accepts());
//     intersection.read("a");
//     EXPECT_FALSE(intersection.accepts());
//     intersection.read("a");
//     EXPECT_FALSE(intersection.accepts());
//     intersection.read("a");
//     EXPECT_FALSE(intersection.accepts());
//     intersection.read("a");
//     EXPECT_FALSE(intersection.accepts());
//     intersection.read("a");
//     EXPECT_TRUE(intersection.accepts());
// }

// TEST_F(TestDFA, Emptyness) {
//     EXPECT_TRUE(instance.empty());
//     instance << "q0";
//     EXPECT_TRUE(instance.empty());
//     instance.accepts("q0");
//     EXPECT_TRUE(instance.empty());
//     instance.addTransition("q0", "q0", 'a');
//     EXPECT_FALSE(instance.empty());
// }

// TEST_F(TestDFA, Equivalence) {
//     instance << "q0";
//     instance << "q1";
//     instance.addTransition("q0", "q1", 'a');
//     instance.accept("q1");
//     EXPECT_EQ(!!instance, instance);
// }

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
