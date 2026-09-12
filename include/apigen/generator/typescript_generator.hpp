#pragma once

#include "apigen/generator/generator.hpp"

namespace apigen {

class TypeScriptGenerator : public IGenerator {
public:
    std::string generate(
        const HttpDocument& document
    ) override;
};

}
