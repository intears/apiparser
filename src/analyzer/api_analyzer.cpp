#include "apigen/analyzer/api_analyzer.hpp"
#include "apigen/analyzer/url_parser.hpp"
#include "apigen/api/definition.hpp"
#include <algorithm>
namespace apigen {

ApiDefinition APIAnalyzer::analyze(const HttpDocument &document) {
  ApiDefinition definition;

  for (const auto &transaction : document.transactions) {
    // 1. Parse the request URL.
    const auto url = apigen::parseUrl(transaction.request.url);

    // 2. Set the API base URL from the first transaction.
    if (definition.baseUrl.empty()) {
      definition.baseUrl = url.scheme + "://" + url.host;
    }

    // 3. Find or create the endpoint.
    auto &endpoint =
        findOrCreateEndpoint(definition, transaction.request.method, url.path);

    // 4. Accumulate query parameter examples.
    addQueryParameters(endpoint, url);

    // 5. Record the request.
    ApiRequest request;
    request.contentType = transaction.request.contentType;
    request.body = transaction.request.body;

    endpoint.requests.push_back(std::move(request));

    // 6. Accumulate the response under its status code.
    if (transaction.response.has_value()) {
      const auto &response = transaction.response.value();

      auto &apiResponse = findOrCreateResponse(endpoint, response.statusCode);

      apiResponse.contentType = response.contentType;

      if (response.body.has_value()) {
        const auto &body = response.body.value();

        // Keep distinct response examples.
        if (std::find(apiResponse.examples.begin(), apiResponse.examples.end(),
                      body) == apiResponse.examples.end()) {
          apiResponse.examples.push_back(body);
        }
      }
    }
  }

  return definition;
}

ApiResponse &APIAnalyzer::findOrCreateResponse(ApiEndpoint &endpoint,
                                               int statusCode) {
  for (auto &response : endpoint.responses) {
    if (response.statusCode == statusCode) {
      return response;
    }
  }

  ApiResponse response;

  response.statusCode = statusCode;

  endpoint.responses.push_back(std::move(response));

  return endpoint.responses.back();
}

ApiEndpoint &APIAnalyzer::findOrCreateEndpoint(ApiDefinition &definition,
                                               std::string_view method,
                                               std::string_view path) {
  for (auto &endpoint : definition.endpoints) {
    if (endpoint.method == method && endpoint.path == path) {
      return endpoint;
    }
  }

  ApiEndpoint newEndpoint;

  newEndpoint.method = method;
  newEndpoint.path = path;

  definition.endpoints.push_back(std::move(newEndpoint));

  return definition.endpoints.back();
}

ApiParameter *APIAnalyzer::findQueryParameter(ApiEndpoint &endpoint,
                                              std::string_view name) {
  for (auto &parameter : endpoint.queryParameters) {
    if (parameter.name == name) {
      return &parameter;
    }
  }

  return nullptr;
}

void APIAnalyzer::addQueryParameters(ApiEndpoint &endpoint,
                                     const ParsedUrl &url) {
  for (const auto &[name, values] : url.queryParameters) {

    auto *parameter = findQueryParameter(endpoint, name);

    if (parameter == nullptr) {
      ApiParameter newParameter;

      newParameter.name = name;
      newParameter.location = ParameterLocation::Query;
      newParameter.examples = values;

      endpoint.queryParameters.push_back(std::move(newParameter));

      continue;
    }

    // Parameter already exists, so add new examples.
    // Avoid duplicates.
    for (const auto &value : values) {
      if (std::find(parameter->examples.begin(), parameter->examples.end(),
                    value) == parameter->examples.end()) {

        parameter->examples.push_back(value);
      }
    }
  }
}

} // namespace apigen
