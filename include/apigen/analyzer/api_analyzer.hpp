#pragma once

#include "apigen/api/definition.hpp"
#include "apigen/api/endpoint.hpp"
#include "apigen/core/types.hpp"

namespace apigen {

class APIAnalyzer {
public:
    ApiDefinition analyze(const HttpDocument& document);
    std::string parseUrl(const HttpTransaction& document);
    ApiEndpoint createEndpoint(const HttpTransaction& transaction);
};

}


