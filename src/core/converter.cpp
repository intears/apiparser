#include "apigen/core/converter.hpp"

#include "apigen/parser/har_parser.hpp"
#include "apigen/generator/typescript_generator.hpp"

namespace apigen {
    std::string Converter::convert(
    const std::filesystem::path& input,
    const std::string& language
    ){
        HarParser parser;

        auto document = parser.parse(input);

        if (language == "typescript") {
            TypeScriptGenerator generator;
            return generator.generate(document);
        }

        throw std::runtime_error (
            "Unsupported Language: " + language
        );

    }
}
