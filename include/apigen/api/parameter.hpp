#pragma once

#include <string>
#include <vector>

namespace apigen {

enum class ParameterLocation {
  Query,
  Path,
  Header,
};

struct ApiParameter {
  std::string name;

  /**
   * The location of this parameter
   * /users/:id
   *    ↑
   *  Path
   *
   * /users?sort=name
   *        ↑
   *      Query
   *
   * Authorization: Bearer ...
   * ↑
   * Header
   */
  ParameterLocation location;

  std::vector<std::string> examples;

  // TODO: later add these
  // ParameterType type;
  // bool required;
  // std::optional<Schema> schema;
};
} // namespace apigen
