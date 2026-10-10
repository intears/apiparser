#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace apigen {

struct ParsedUrl {
  std::string scheme;
  std::string host;
  std::string path;

  std::map<std::string, std::vector<std::string>> queryParameters;
};

/**
 * @description generate the url data from the string passed
 * @param url std::string_view the url we want to parse into our object
 * @return ParsedUrl the url object that we want to have parsed
 *
 */
ParsedUrl parseUrl(std::string_view url);

/**
 * @description parse query parameters from a string
 * @param query std::string_view the string that we want to parse from
 * @return std::map<std::string, std::vector<std::string>> the list of params in
 * a map
 *
 */
std::map<std::string, std::vector<std::string>>
parseQueryParameters(std::string_view query);

} // namespace apigen
