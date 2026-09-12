#include "apigen/parser/har_parser.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace apigen {

HttpDocument HarParser::parse(
    const std::filesystem::path& file
) {
    std::ifstream input(file);

    if (!input) {
        throw std::runtime_error(
            "Failed to open HAR file: " + file.string()
        );
    }

    nlohmann::json har;

    try {
        input >> har;
    }
    catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error(
            "Invalid JSON in HAR file: " +
            std::string(e.what())
        );
    }

    if (!har.contains("log")) {
        throw std::runtime_error(
            "Invalid HAR file: missing 'log'"
        );
    }

    if (!har["log"].contains("entries")) {
        throw std::runtime_error(
            "Invalid HAR file: missing 'log.entries'"
        );
    }

    if (!har["log"]["entries"].is_array()) {
        throw std::runtime_error(
            "Invalid HAR file: 'log.entries' must be an array"
        );
    }

    HttpDocument document;

    for (const auto& entry : har["log"]["entries"]) {
        if (!entry.contains("request")) {
            throw std::runtime_error(
                "Invalid HAR entry: missing 'request'"
            );
        }

        const auto& request = entry["request"];

        HttpTransaction transaction;

        transaction.request.method =
            request["method"].get<std::string>();

        transaction.request.url =
            request["url"].get<std::string>();

        for (const auto& header : request["headers"]) {
            transaction.request.headers[
                header["name"].get<std::string>()
            ] = header["value"].get<std::string>();
        }

        document.transactions.push_back(
            std::move(transaction)
        );
    }

    return document;
}

}
