#include <gtest/gtest.h>
#include <cacos/util/split.h>

TEST(split, simple) {
    std::vector<std::string_view> v1 = cacos::util::split("qwe:qwe::qwe:", ":");
    std::vector<std::string_view> v2 {"qwe", "qwe", "", "qwe", ""};
    EXPECT_EQ(v1, v2);
}

TEST(split, delimiters) {
    static const std::string s =
        "Hello, world! .., what. ,q we qwe  ,, qwqwe qwe 123qwaq 12k,12.123  213, 123123.1!? ";
    std::vector<std::string_view> v1 = cacos::util::split(s, " ,.??");
    std::vector<std::string_view> v2 {
        "Hello", "", "world!", "",
        "", "", "", "what", "", "",
        "q", "we", "qwe", "", "", "",
        "", "qwqwe", "qwe", "123qwaq",
        "12k", "12", "123", "", "213",
        "", "123123", "1!", "", ""
    };
    EXPECT_EQ(v1, v2);
}

TEST(split, empty) {
    std::vector<std::string_view> v1 = cacos::util::split("", ":,");
    std::vector<std::string_view> v2 {""};
    EXPECT_EQ(v1, v2);
}

TEST(split, no_delimiters) {
    std::vector<std::string_view> v1 = cacos::util::split("Hello, world!", "$");
    std::vector<std::string_view> v2 {"Hello, world!"};
    EXPECT_EQ(v1, v2);
}

TEST(split, only_delimiters) {
    std::vector<std::string_view> v1 = cacos::util::split("$$$!$", "$!");
    std::vector<std::string_view> v2 {"", "", "", "", "", ""};
    EXPECT_EQ(v1, v2);
}
