#include "apigen/core/string_utils.hpp"

namespace apigen {

bool equalsIgnoreCase(std::string_view a, std::string_view b) {
    // check to see if they are the same length
    if (a.size() != b.size()) {
        return false;
    }

    // check to see fi they are the chars one by one
    if (std::equal(a.begin(), a.end(), b.begin(),
                [](unsigned char CharA, unsigned char CharB) {
                    return std::tolower(CharA) == std::tolower(CharB);
                })
       ) {
        return true;
    }

    return false;
}

}
