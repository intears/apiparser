#pragma once

#include "apigen/analyzer/url_parser.hpp"
#include "apigen/api/definition.hpp"
#include "apigen/api/endpoint.hpp"
#include "apigen/core/types.hpp"

namespace apigen {

class APIAnalyzer {
public:
    ApiDefinition analyze(const HttpDocument& document);
    //std::string parseUrl(const HttpTransaction& document);
    //ApiEndpoint createEndpoint(const HttpTransaction& transaction);
private:
    ApiEndpoint& findOrCreateEndpoint(ApiDefinition& definition, std::string_view method, std::string_view path);
    ApiResponse& findOrCreateResponse(ApiEndpoint& endpoint, int statusCode);
        void addQueryParameters(
        ApiEndpoint& endpoint,
        const ParsedUrl& url
    );

    ApiParameter* findQueryParameter(
        ApiEndpoint& endpoint,
        std::string_view name
    );
};

}


