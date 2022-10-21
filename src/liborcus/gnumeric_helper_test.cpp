#include "gnumeric_helper.hpp"
#include "gnumeric_helper.cpp"

#include <iostream>

namespace {

void test_split_string()
{
    std::string_view str("str1:str2:str3");
    auto res = orcus::string_helper::split_string(str, ':');
    assert(res.size() == 3);
    assert(res[0] == "str1");
    assert(res[1] == "str2");
    assert(res[2] == "str3");
}

void test_parse_color_string()
{
    std::string_view str("8080");
    size_t res = orcus::parse_color_string(str);
    std::cout << res << std::endl;
    assert(res == 128);

    res = orcus::parse_color_string("FFFF");
    assert(res == 255);

    res = orcus::parse_color_string("0");
    assert(res == 0);
}

}

int main()
{
    test_parse_color_string();
    test_split_string();

    return EXIT_SUCCESS;
}
