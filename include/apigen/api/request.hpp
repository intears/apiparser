#pragma once

#include <optional>
#include <string>

namespace apigen {

struct ApiRequest {
    std::optional<std::string> contentType;

    std::optional<std::string> body;

    // future
    // std::vector<PathParameter> pathParameters;
    // std::optional<Schema> schema;
};

}
