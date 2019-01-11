#include <gtest/gtest.h>
#include <cacos/util/inline_variables.h>

TEST(inline_variables, simple) {
    cacos::InlineVariablesParser parser;
    parser.add("source", "main.cpp");
    parser.add("binary", "main.exe");

    std::string s = parser.parse("Hello, world! What about clang++ @{source} -o @{binary}?");
    EXPECT_EQ(s, "Hello, world! What about clang++ main.cpp -o main.exe?");
}

TEST(inline_variables, throws) {
    cacos::InlineVariablesParser parser(cacos::UnknownVariablePolicy::THROW);
    parser.add("source", "main.cpp");
    parser.add("binary", "main.exe");

    EXPECT_THROW(parser.parse("Hello, world! What about clang++ @{soqurce} -o @{binary}?"),
        cacos::UnknownVariableName);
}

TEST(inline_variables, empty) {
    cacos::InlineVariablesParser parser;
    parser.add("source", "main.cpp");
    parser.add("binary", "main.exe");

    std::string s = parser.parse("Hello, world! What about clang++ @{soqurce} -o @{binary}?");
    EXPECT_EQ(s, "Hello, world! What about clang++  -o main.exe?");
}
