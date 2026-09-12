#pragma once

#include <filesystem>
#include "apigen/core/types.hpp"

namespace apigen {

class IParser {
public:
    virtual ~IParser() = default;

    virtual HttpDocument parse(
            const std::filesystem::path& file
        ) = 0;
};


}
