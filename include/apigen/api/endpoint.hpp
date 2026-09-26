#pragma once

#include "apigen/api/parameter.hpp"
#include "apigen/api/request.hpp"
#include "apigen/api/response.hpp"
#include <string>

namespace apigen {

struct ApiEndpoint {
  std::string method;
  std::string path;

  std::vector<ApiParameter> queryparameters;

  std::vector<ApiRequest> requests;
  std::vector<ApiResponse> responses;
};

} // namespace apigen
