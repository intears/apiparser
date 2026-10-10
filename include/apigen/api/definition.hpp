#pragma once

#include <string>
#include <vector>

#include "apigen/api/endpoint.hpp"

namespace apigen {

struct ApiDefinition {
    std::string baseUrl;
    std::vector<ApiEndpoint> endpoints;
};


}
