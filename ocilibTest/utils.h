//-------------------------------------------------------------------------
#ifndef __UTILS_H__
#define __UTILS_H__
//-------------------------------------------------------------------------
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>
#include <variant>
#include <array>
#include <algorithm>
//-------------------------------------------------------------------------
namespace std 
{
    const auto to_lower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
        return s;
        };


    const auto to_upper = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return toupper(c); });
        return s;
        };
}  // namespace std

//-------------------------------------------------------------------------
#endif // !__UTILS_H__
//-------------------------------------------------------------------------
