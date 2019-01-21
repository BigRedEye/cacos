#include <gtest/gtest.h>
#include <cacos/util/inline_variables.h>

TEST(inline_variables, simple) {
    cacos::InlineVariables parser;
    parser.set("source", "main.cpp");
    parser.set("binary", "main.exe");

    std::string s = parser.parse("Hello, world! What about clang++ @{source} -o @{binary}?");
    EXPECT_EQ(s, "Hello, world! What about clang++ main.cpp -o main.exe?");
}

TEST(inline_variables, throws) {
    cacos::InlineVariables parser(cacos::UnknownVariablePolicy::THROW);
    parser.set("source", "main.cpp");
    parser.set("binary", "main.exe");

    EXPECT_THROW(parser.parse("Hello, world! What about clang++ @{soqurce} -o @{binary}?"),
        cacos::UnknownVariableName);
}

TEST(inline_variables, empty) {
    cacos::InlineVariables parser;
    parser.set("source", "main.cpp");
    parser.set("binary", "main.exe");

    std::string s = parser.parse("Hello, world! What about clang++ @{soqurce} -o @{binary}?");
    EXPECT_EQ(s, "Hello, world! What about clang++  -o main.exe?");
}

TEST(inline_variables, multiple) {
    cacos::InlineVariables parser;
    parser.set("@", "a");
    parser.set("@@", "b");
    parser.set("a", "c");
    parser.set("b", "d");

    std::string s = parser.parse("@{a}@{b} qwe123? 123 @{@}@{@}@{@}  @{@{}@{}");
    EXPECT_EQ(s, "cd qwe123? 123 aaa  ");
}

TEST(inline_variables, prefix) {
    cacos::InlineVariables parser("prefix");
    parser.set("@", "a");
    parser.set("@@", "b");
    parser.set("a", "c");
    parser.set("b", "d");
    parser.set("{", "e");

    std::string s = parser.parse("prefix{a}@{b} qwe123? 123 q@{@}prefix@{@}prefix{@}  @{prefix{{}");
    EXPECT_EQ(s, "c@{b} qwe123? 123 q@{@}prefix@{@}a  @{e");
}

