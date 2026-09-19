#pragma once

#include "apigen/api/request.hpp"
#include "apigen/api/response.hpp"
#include <map>
#include <string>

namespace apigen {

struct ApiEndpoint {
    std::string method;
    std::string path;

    std::map<std::string, std::string> queryParamaters;

    std::vector<ApiRequest> requests;

    std::vector<ApiResponse> responses;
};


}
