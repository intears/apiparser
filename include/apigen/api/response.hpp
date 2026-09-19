#pragma once

#include <optional>
#include <string>
#include <vector>

namespace apigen {

struct ApiResponse {
    int statusCode = 0;

    std::optional<std::string> contentType;

    /*
     *
     * Keep multiple examples of request because
     * the har may contain multiple observations
     * of the same response.
     *
     */
    std::vector<std::string> examples;


    // Future:
    // std::optional<Schema> schema;


};


}
