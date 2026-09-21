#pragma once

#include "apigen/api/definition.hpp"
#include "apigen/api/endpoint.hpp"
#include "apigen/core/types.hpp"

namespace apigen {

class APIAnalyzer {
public:
    ApiDefinition analyze(const HttpDocument& document);
    std::string parseUrl(const HttpTransaction& document);
    //ApiEndpoint createEndpoint(const HttpTransaction& transaction);
private:
    ApiEndpoint& findOrCreateEndpoint(ApiDefinition& definition, std::string_view method, std::string_view path);
    ApiResponse& findOrCreateResponse(ApiEndpoint& endpoint, int statusCode);
};

}


