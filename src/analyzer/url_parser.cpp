#include "apigen/analyzer/url_parser.hpp"
#include "apigen/core/error.hpp"

namespace apigen {

ParsedUrl parseUrl(std::string_view url)
{
    ParsedUrl result;

    const auto schemeEnd = url.find("://");

    if (schemeEnd == std::string_view::npos) {
        throw UrlParseError("Invalid URL: missing scheme");
    }

    if (url.substr(0, schemeEnd) == "") {
        throw UrlParseError("Invalid URL: missing scheme");
    }

    result.scheme = std::string(
        url.substr(0, schemeEnd)
    );

    const auto hostStart = schemeEnd + 3;

    const auto pathStart = url.find(
        '/',
        hostStart
    );


    if (pathStart == std::string_view::npos) {
        result.host = std::string(
            url.substr(hostStart)
        );

        result.path = "/";

        return result;
    }

    result.host = std::string(
        url.substr(
            hostStart,
            pathStart - hostStart
        )
    );

    const auto queryStart = url.find(
        '?',
        pathStart
    );

    if (queryStart == std::string_view::npos) {
        result.path = std::string(
            url.substr(pathStart)
        );
    } else {
        result.path = std::string(
            url.substr(
                pathStart,
                queryStart - pathStart
            )
        );
    }



    return result;
}


}
