#pragma once

#include "apigen/parser/parser.hpp"

namespace apigen {

class HarParser : public IParser {
public:
    /**
     * @description Parse a .HAR (Http Archive) File into a HttpDocument
     * @param file std::filesystem::path& the path to the HAR file that we want to parse
     * @throws std::runtime_error if file cant be found.
     * @return HttpDocument the data in a core class format this library uses
     */
    HttpDocument parse(
            const std::filesystem::path& file
        ) override;
};

}
