#include "apigen/analyzer/api_analyzer.hpp"
#include "apigen/analyzer/url_parser.hpp"
#include "apigen/api/definition.hpp"
#include <algorithm>
#include <cctype>
#include <string_view>
#include <utility>
#include <vector>

namespace apigen {

struct PathParameterValue {
  std::string name;
  std::string value;
};

struct NormalizedPath {
  std::string path;
  std::vector<PathParameterValue> parameters;
};

bool isNumericSegment(std::string_view segment) {
  if (segment.empty()) {
    return false;
  }

  for (const char character : segment) {
    if (!std::isdigit(static_cast<unsigned char>(character))) {
      return false;
    }
  }

  return true;
}

bool isUuidSegment(std::string_view segment) {
  if (segment.size() != 36) {
    return false;
  }

  for (std::size_t i = 0; i < segment.size(); ++i) {
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      if (segment[i] != '-') {
        return false;
      }
    } else if (!std::isxdigit(static_cast<unsigned char>(segment[i]))) {
      return false;
    }
  }

  return true;
}

NormalizedPath normalizePath(std::string_view path) {
  NormalizedPath result;

  std::size_t start = 0;
  std::size_t parameterIndex = 0;

  while (start < path.size()) {
    // preserve the spash separating path segments
    if (path[start] == '/') {
      result.path += '/';
      ++start;
      continue;
    }

    const auto end = path.find('/', start);
    const auto length =
        end == std::string_view::npos ? path.size() - start : end - start;

    const auto segment = path.substr(start, length);

    if (isNumericSegment(segment) || isUuidSegment(segment)) {
      const std::string name = parameterIndex == 0
                                   ? "id"
                                   : "id" + std::to_string(parameterIndex + 1);

      result.path += "{" + name + "}";
      result.parameters.push_back({name, std::string(segment)});

      ++parameterIndex;
    } else {
      result.path += segment;
    }

    start += length;
  }

  return result;
}

ApiDefinition APIAnalyzer::analyze(const HttpDocument &document) {
  ApiDefinition definition;

  for (const auto &transaction : document.transactions) {
    // 1. Parse the request URL.
    const auto url = apigen::parseUrl(transaction.request.url);

    // 2. Set the API base URL from the first transaction.
    if (definition.baseUrl.empty()) {
      definition.baseUrl = url.scheme + "://" + url.host;
    }

    // 3.normalize the path and  Find or create the endpoint.
    const auto normalizedPath = normalizePath(url.path);
    auto &endpoint =
        findOrCreateEndpoint(definition, transaction.request.method, normalizedPath.path);

    // 4. Accumulate query parameter examples.
    addQueryParameters(endpoint, url);

    // 5. Accumulate path parameters
    for (const auto &observed : normalizedPath.parameters) {
      auto parameter = std::find_if(
          endpoint.pathParameters.begin(), endpoint.pathParameters.end(),
          [&](const ApiParameter &item) { return item.name == observed.name; });

      if (parameter == endpoint.pathParameters.end()) {
        ApiParameter newParameter;
        newParameter.name = observed.name;
        newParameter.location = ParameterLocation::Path;
        newParameter.examples.push_back(observed.value);

        endpoint.pathParameters.push_back(std::move(newParameter));
      } else if (std::find(parameter->examples.begin(),
                           parameter->examples.end(),
                           observed.value) == parameter->examples.end()) {
        parameter->examples.push_back(observed.value);
      }
    }

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
