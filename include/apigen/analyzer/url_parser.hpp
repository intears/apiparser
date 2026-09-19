#pragma once

#include <string>
#include <string_view>

namespace apigen {

struct ParsedUrl {
    std::string scheme;
    std::string host;
    std::string path;
};

/**
 * @description generate the url data from the string passed
 * @@param url std::string_view the url we want to parse into our object
 * @@return ParsedUrl the url object that we want to have parsed
 *
 */
ParsedUrl parseUrl(std::string_view url);

}
