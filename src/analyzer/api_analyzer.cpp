#include "apigen/analyzer/api_analyzer.hpp"
#include "apigen/analyzer/url_parser.hpp"
#include "apigen/api/definition.hpp"
#include <algorithm>
#include <cctype>
#include <string_view>
#include <utility>
#include <vector>
#include <set>

namespace apigen {
namespace {
struct PathParameterValue {
  std::string name;
  std::string value;
};

struct NormalizedPath {
  std::string path;
  std::vector<PathParameterValue> parameters;
};

/**
 * @description creates the param name for the resource.
 * This will take the /user/id and make it return {userId}.
 * This will also remove plurals and camelCase the entry
 *
 * @param std::string_view resource the resource name that we want to standardize
 *
 * @return std::string the new resource name after fixing
 *
 */
std::string parameterNameForResource(std::string_view resource) {
  if (resource.empty()) {
    return "id";
  }

  // convert names like user-profiles or user_profiles
  // to camelCase vesrison: userProfiles
  std::string name;
  bool capitalizeNext = false;

  for (const char character : resource) {
    if (character == '-' || character == '_') {
      capitalizeNext = true;
      continue;
    }

    const auto c = static_cast<unsigned char>(character);

    if (name.empty()) {
      name += static_cast<char>(std::tolower(c));
    } else if (capitalizeNext) {
      name += static_cast<char>(std::toupper(c));
    } else {
      name += static_cast<char>(std::tolower(c));
    }

    capitalizeNext = false;
  }

  // Singularize common plural resource names.
  if (name.size() > 3 && name.compare(name.size() - 3, 3, "ies") == 0) {
    name.replace(name.size() - 3, 3, "y");
  } else if (name.size() > 4 &&
             (name.ends_with("sses") || name.ends_with("shes") ||
              name.ends_with("ches") || name.ends_with("xes") ||
              name.ends_with("zes"))) {
    name.erase(name.size() - 2);
  } else if (name.size() > 2 && name.ends_with("s") && !name.ends_with("ss") &&
             !name.ends_with("us") && !name.ends_with("is")) {
    name.pop_back();
  }

  return name.empty() ? "id" : name + "Id";
}

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

    std::string previousSegment;
    std::vector<std::string> usedNames;

    std::size_t start = 0;

    while (start < path.size()) {
        if (path[start] == '/') {
            result.path += '/';
            ++start;
            continue;
        }

        const auto end = path.find('/', start);
        const auto length =
            end == std::string_view::npos
                ? path.size() - start
                : end - start;

        const auto segment = path.substr(start, length);

        if (isNumericSegment(segment) || isUuidSegment(segment)) {
            std::string name = parameterNameForResource(previousSegment);

            // Avoid repeating a parameter name in the same path.
            const std::string baseName = name;
            std::size_t suffix = 2;

            while (std::find(
                       usedNames.begin(),
                       usedNames.end(),
                       name
                   ) != usedNames.end()) {
                name = baseName + std::to_string(suffix++);
            }

            usedNames.push_back(name);

            result.path += "{" + name + "}";
            result.parameters.push_back({
                name,
                std::string(segment)
            });
        } else {
            result.path += segment;
            previousSegment = std::string(segment);
        }

        start += length;
    }

    return result;
}


using PathSegments = std::vector<std::string>;

PathSegments splitPath(std::string_view path) {
    PathSegments segments;
    std::size_t start = 0;

    while (start < path.size()) {
        while (start < path.size() && path[start] == '/') {
            ++start;
        }

        if (start == path.size()) {
            break;
        }

        const auto end = path.find('/', start);
        const auto length = end == std::string_view::npos
            ? path.size() - start
            : end - start;

        segments.emplace_back(path.substr(start, length));
        start += length;
    }

    return segments;
}

std::string joinPath(const PathSegments& segments) {
    std::string path = "/";

    for (std::size_t i = 0; i < segments.size(); ++i) {
        if (i > 0) {
            path += '/';
        }

        path += segments[i];
    }

    return path;
}

bool isStaticRouteSegment(std::string_view segment) {
    static const std::set<std::string> staticRoutes = {
        "me", "self", "settings", "search", "create",
        "delete", "new", "current", "login", "logout",
        "register", "refresh", "count", "stats"
    };

    return staticRoutes.contains(std::string(segment));
}

// Build a comparison key that ignores the candidate segment.
// Numeric and UUID segments elsewhere are also treated as dynamic.
std::string makeComparisonKey(
    std::string_view method,
    const PathSegments& segments,
    std::size_t candidateIndex
) {
    std::string key(method);
    key += "|" + std::to_string(segments.size());
    key += "|" + std::to_string(candidateIndex);

    for (std::size_t i = 0; i < segments.size(); ++i) {
        key += "|";

        if (i == candidateIndex ||
            isNumericSegment(segments[i]) ||
            isUuidSegment(segments[i])) {
            key += "*";
        } else {
            key += std::to_string(segments[i].size());
            key += ":";
            key += segments[i];
        }
    }

    return key;
}


} // namespace

ApiDefinition APIAnalyzer::analyze(const HttpDocument& document) {
    struct Observation {
        const HttpTransaction* transaction;
        ParsedUrl url;
        PathSegments segments;
    };

    ApiDefinition definition;
    std::vector<Observation> observations;
    observations.reserve(document.transactions.size());

    // Pass 1: collect the raw paths.
    for (const auto& transaction : document.transactions) {
        auto url = parseUrl(transaction.request.url);

        observations.push_back({
            &transaction,
            url,
            splitPath(url.path)
        });

        if (definition.baseUrl.empty()) {
            definition.baseUrl = url.scheme + "://" + url.host;
        }
    }

    // Record the distinct values observed at each candidate position.
    std::map<std::string, std::set<std::string>> observedValues;

    for (const auto& observation : observations) {
        const auto& method = observation.transaction->request.method;

        for (std::size_t i = 0; i < observation.segments.size(); ++i) {
            const auto& segment = observation.segments[i];

            // Numeric and UUID segments are already recognized.
            if (isNumericSegment(segment) ||
                isUuidSegment(segment) ||
                isStaticRouteSegment(segment)) {
                continue;
            }

            const auto key = makeComparisonKey(
                method,
                observation.segments,
                i
            );

            observedValues[key].insert(segment);
        }
    }

    // Pass 2: normalize paths and aggregate endpoint observations.
    for (const auto& observation : observations) {
        const auto& transaction = *observation.transaction;
        const auto& method = transaction.request.method;

        PathSegments normalizedSegments;
        NormalizedPath normalizedPath;
        std::string previousSegment;
        std::vector<std::string> usedNames;

        for (std::size_t i = 0; i < observation.segments.size(); ++i) {
            const auto& segment = observation.segments[i];

            bool isParameter =
                isNumericSegment(segment) || isUuidSegment(segment);

            if (!isParameter && !isStaticRouteSegment(segment)) {
                const auto key = makeComparisonKey(
                    method,
                    observation.segments,
                    i
                );

                const auto it = observedValues.find(key);

                isParameter =
                    it != observedValues.end() &&
                    it->second.size() > 1;
            }

            if (isParameter) {
                std::string name =
                    parameterNameForResource(previousSegment);

                const std::string baseName = name;
                std::size_t suffix = 2;

                while (std::find(
                           usedNames.begin(),
                           usedNames.end(),
                           name
                       ) != usedNames.end()) {
                    name = baseName + std::to_string(suffix++);
                }

                usedNames.push_back(name);
                normalizedSegments.push_back("{" + name + "}");

                normalizedPath.parameters.push_back({
                    name,
                    segment
                });
            } else {
                normalizedSegments.push_back(segment);
                previousSegment = segment;
            }
        }

        normalizedPath.path = joinPath(normalizedSegments);

        auto& endpoint = findOrCreateEndpoint(
            definition,
            method,
            normalizedPath.path
        );

        addQueryParameters(endpoint, observation.url);

        // Accumulate path parameter examples.
        for (const auto& observed : normalizedPath.parameters) {
            auto parameter = std::find_if(
                endpoint.pathParameters.begin(),
                endpoint.pathParameters.end(),
                [&](const ApiParameter& item) {
                    return item.name == observed.name;
                }
            );

            if (parameter == endpoint.pathParameters.end()) {
                ApiParameter newParameter;
                newParameter.name = observed.name;
                newParameter.location = ParameterLocation::Path;
                newParameter.examples.push_back(observed.value);

                endpoint.pathParameters.push_back(std::move(newParameter));
            } else if (
                std::find(
                    parameter->examples.begin(),
                    parameter->examples.end(),
                    observed.value
                ) == parameter->examples.end()
            ) {
                parameter->examples.push_back(observed.value);
            }
        }

        // Record the request.
        ApiRequest request;
        request.contentType = transaction.request.contentType;
        request.body = transaction.request.body;

        endpoint.requests.push_back(std::move(request));

        // Aggregate responses by status code.
        if (transaction.response.has_value()) {
            const auto& response = transaction.response.value();

            auto& apiResponse = findOrCreateResponse(
                endpoint,
                response.statusCode
            );

            apiResponse.contentType = response.contentType;

            if (response.body.has_value()) {
                const auto& body = response.body.value();

                if (std::find(
                        apiResponse.examples.begin(),
                        apiResponse.examples.end(),
                        body
                    ) == apiResponse.examples.end()) {
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
