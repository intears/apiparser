#pragma once

#include <filesystem>
#include <string>

namespace apigen {

class Converter {
public:
    std::string convert(
        const std::filesystem::path& input,
        const std::string& language
    );
};

}
