#include "apigen/parser/har_parser.hpp"
#include "apigen/core/types.hpp"

#include <fstream>
#include <stdexcept>

namespace apigen {

HttpDocument HarParser::parse(const std::filesystem::path &file) {
  std::ifstream input(file);

  if (!input) {
    throw std::runtime_error("Failed to open HAR file: " + file.string());
  }

  nlohmann::json har;

  try {
    input >> har;
  } catch (const nlohmann::json::parse_error &e) {
    throw std::runtime_error("Invalid JSON in HAR file: " +
                             std::string(e.what()));
  }

  if (!har.contains("log")) {
    throw std::runtime_error("Invalid HAR file: missing 'log'");
  }

  if (!har["log"].contains("entries")) {
    throw std::runtime_error("Invalid HAR file: missing 'log.entries'");
  }

  if (!har["log"]["entries"].is_array()) {
    throw std::runtime_error(
        "Invalid HAR file: 'log.entries' must be an array");
  }

  HttpDocument document;

  for (const auto &entry : har["log"]["entries"]) {

    if (!entry.contains("request")) {
      throw std::runtime_error("Invalid HAR entry: missing 'request'");
    }


    document.transactions.push_back(parseTransaction(entry));
  }

  return document;
}

HttpTransaction HarParser::parseTransaction(const nlohmann::json &entry) {

    HttpTransaction transaction;

    transaction.request = parseRequest(entry["request"]);

    if (entry.contains("response")) {
        transaction.response = parseResponse(entry["response"]);
    }
    return transaction;
}

HttpRequest HarParser::parseRequest(const nlohmann::json &request) {

    HttpRequest result;

    result.method = request["method"].get<std::string>();
    result.url = request["url"].get<std::string>();

    if (request.contains("headers")) {
        result.headers =
            parseHeaders(request["headers"]);
    }

    if (request.contains("queryString")) {
        result.query =
            parseQuery(request["queryString"]);
    }
    return result;
}

HttpResponse HarParser::parseResponse(const nlohmann::json &response) {
    HttpResponse result;

    if (response.contains("headers")) {
        result.headers =
            parseHeaders(response["headers"]);
    }

    return result;

}

std::map<std::string, std::string> HarParser::parseHeaders(const nlohmann::json &headers){

    std::map<std::string, std::string> result;


    for (const auto& header : headers) {
      result[header["name"].get<std::string>()] =
          header["value"].get<std::string>();
    }

    return result;
}

std::map<std::string, std::string>
HarParser::parseQuery(const nlohmann::json &queryString){
    std::map<std::string, std::string> result;

    for (const auto& param : queryString) {
        result[
            param["name"].get<std::string>()
        ] = param["value"].get<std::string>();

    }

    return result;
}

} // namespace apigen
