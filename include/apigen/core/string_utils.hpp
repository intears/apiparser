#pragma once

#include <string>
#include <string_view>


namespace apigen {

/**
 * @description compares the 2 strings
 * doesnt check for case
 *
 * @param a std::string_view the first string
 * @param b std::string_view the second string
 *
 * @return bool on if they are the same value
 *
 */
bool equalsIgnoreCase(
    std::string_view a,
    std::string_view b
);


}
