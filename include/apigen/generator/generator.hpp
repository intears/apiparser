#pragma once

#include <string>

#include "apigen/core/types.hpp"

namespace apigen {

class IGenerator {
public:
    virtual ~IGenerator() = default;

    virtual std::string generate(
        const HttpDocument& document
    ) = 0;
};

}
