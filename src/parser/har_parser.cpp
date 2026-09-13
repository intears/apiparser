#include "apigen/parser/har_parser.hpp"
#include "apigen/core/string_utils.hpp"
#include "apigen/core/types.hpp"
#include "apigen/core/errors.hpp"

#include <fstream>
#include <optional>
#include <stdexcept>

namespace apigen {

HttpDocument HarParser::parse(const std::filesystem::path &file) {
  std::ifstream input(file);

  if (!input) {
    throw apigen::InvalidHarError(ErrorCode::FileNotFound, "Failed to open HAR file: " + file.string());
  }

  nlohmann::json har;

  try {
    input >> har;
  } catch (const nlohmann::json::parse_error &e) {
    throw apigen::InvalidJsonError(ErrorCode::InvalidJson, "Invalid JSON in HAR file: " + std::string(e.what()));
  }

  if (!har.contains("log")) {
    throw apigen::InvalidHarError(ErrorCode::MissingHarLog, "Invalid HAR file: missing 'log'");
  }

  if (!har["log"].contains("entries")) {
    throw apigen::InvalidHarError(ErrorCode::MissingHarEntries, "Invalid HAR file: missing 'log.entries'");
  }

  if (!har["log"]["entries"].is_array()) {
    throw apigen::InvalidHarError(ErrorCode::InvalidHar, "Invalid HAR file: 'log.entries' must be an array");
  }

  HttpDocument document;

  for (const auto &entry : har["log"]["entries"]) {

    if (!entry.contains("request")) {
      throw apigen::InvalidHarError(ErrorCode::InvalidHar, "Invalid HAR entry: missing 'request'");
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

    if (request.contains("postData")) {
        result.body = parseBody(request["postData"]);

        const auto contentType = getHeader(result.headers, "Content-Type");

        if (contentType) {
            result.contentType = contentType;
        }
    }
    return result;
}

HttpResponse HarParser::parseResponse(const nlohmann::json &response) {
    HttpResponse result;

    result.statusCode = response["status"].get<int>();
    result.statusText = response["statusText"].get<std::string>();

    if (response.contains("headers")) {
        result.headers =
            parseHeaders(response["headers"]);
    }

    if (response.contains("content")) {
        result.body =
            parseBody(response["content"]);

        const auto contentType = getHeader(result.headers, "Content-Type");
        if (contentType) {
            result.contentType = contentType;
        }
    }

    return result;
}

std::optional<std::string> HarParser::parseBody(const nlohmann::json &body) {
    if (!body.contains("text")) {
        return std::nullopt;
    }

    return body["text"].get<std::string>();
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

std::optional<std::string> HarParser::getHeader(
        const std::map<std::string, std::string>& headers,
        const std::string_view name
        ) {

    for (const auto& [headerName, value] : headers) {
        if (equalsIgnoreCase(headerName, name)) {
            return value;
        }
    //     // check to see if they are the same length
    //     if (headerName.size() != name.size()) {
    //         continue;
    //     }
    //
    //     // check to see fi they are the chars one by one
    //     if (std::equal(headerName.begin(), headerName.end(), name.begin(),
    //                 [](unsigned char CharA, unsigned char CharB) {
    //                     return std::tolower(CharA) == std::tolower(CharB);
    //                 })
    //        ) {
    //         return value;
    //     }
    }
    return std::nullopt;
}
} // namespace apigen
