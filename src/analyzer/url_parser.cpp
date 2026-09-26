#include "apigen/analyzer/url_parser.hpp"
#include "apigen/core/error.hpp"
#include <string_view>
#include <vector>

namespace apigen {

ParsedUrl parseUrl(std::string_view url) {
  ParsedUrl result;

  const auto schemeEnd = url.find("://");

  if (schemeEnd == std::string_view::npos) {
    throw UrlParseError("Invalid URL: missing scheme");
  }

  if (url.substr(0, schemeEnd) == "") {
    throw UrlParseError("Invalid URL: missing scheme");
  }

  result.scheme = std::string(url.substr(0, schemeEnd));

  const auto hostStart = schemeEnd + 3;

  const auto pathStart = url.find('/', hostStart);

  if (pathStart == std::string_view::npos) {
    result.host = std::string(url.substr(hostStart));

    result.path = "/";

    return result;
  }

  result.host = std::string(url.substr(hostStart, pathStart - hostStart));

  const auto queryStart = url.find('?', pathStart);

  if (queryStart == std::string_view::npos) {
    result.path = std::string(url.substr(pathStart));
    return result;
  }

  result.path = std::string(url.substr(pathStart, queryStart - pathStart));

  const auto query = url.substr(
          queryStart + 1
    );

  result.queryParameters = parseQueryParameters(query);

  return result;
}

std::map<std::string, std::vector<std::string>> parseQueryParameters(std::string_view query) {
    std::map<std::string, std::vector<std::string>> parameters;

    while (!query.empty()) {
        const auto separator = query.find('&');


        // get one parameter
        // "id=123"
        std::string_view parameter =
            query.substr(
                    0,
                    separator == std::string_view::npos
                    ? query.size()
                    : separator
                    );

        const auto equals = parameter.find('=');

        std::string_view name;
        std::string_view value;

        if (equals == std::string_view::npos) {
            // supports ?debug
            name = parameter;
        } else {
            name = parameter.substr(0, equals);
            value = parameter.substr(equals + 1);
        }

        if (!name.empty()) {
            parameters[std::string(name)]
                .push_back(std::string(value));
        }

        // no more parameters check
        if (separator == std::string_view::npos) {
            break;
        }

        // move past '&'
        query.remove_prefix(separator + 1);

    }

    return parameters;;
}

} // namespace apigen
