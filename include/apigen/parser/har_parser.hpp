#pragma once

#include "apigen/core/types.hpp"
#include "apigen/parser/parser.hpp"

#include <nlohmann/json.hpp>

namespace apigen {

class HarParser : public IParser {
public:
    /**
     * @description Parse a .HAR (Http Archive) File into a HttpDocument
     * @param &file std::filesystem::path the path to the HAR file that we want to parse
     * @throws std::runtime_error if file cant be found.
     * @return HttpDocument the data in a core class format this library uses
     */
    HttpDocument parse(
            const std::filesystem::path& file
        ) override;

private:

    /**
     * @description Parse the transaction of a Har Request
     * This means the HTTP request -> HTTP Response into the core format
     * @param &entry nlohmann::json the Entry on the HTTP request
     * @throws std::runtime_error on failing to parse any of the the transaction
     *
     * @return HttpTransaction of the HTTPtransaction
     */
    HttpTransaction parseTransaction(
            const nlohmann::json& entry
        );
    /**
     * @description Parse the request of the Transaction
     * @param &request nlohmann::json the request JSON object
     * @throws std::runtime_error on failing to parse any of the the request
     *
     * @return HttpRequest of the request
     */
    HttpRequest parseRequest(
            const nlohmann::json& request
        );

    /**
     * @description Parse the response of the Transaction
     * @param &request nlohmann::json the response JSON object
     * @throws std::runtime_error on failing to parse any of the the request
     *
     * @return HttpResponse of the request that we sent
     */
    HttpResponse parseResponse(
            const nlohmann::json& response
        );


    /**
     * @description Parse the headers of the request
     * @param &request nlohmann::json the headers JSON object
     * @throws std::runtime_error failed to parse the header format
     *
     * @return std::map<std::string, std::string> of the headers as [KEY:VALUE]
     */
    std::map<std::string, std::string> parseHeaders(
            const nlohmann::json& headers
        );

    /**
     * @description Parse the query of the request (...?something=test&something2=test2...)
     * @param &request nlohmann::json the queryString of the request
     * @throws std::runtime_error failed to parse the query format
     *
     * @return std::map<std::string, std::string> of the query params as [PARAM:VALUE]
     */
    std::map<std::string, std::string> parseQuery(
            const nlohmann::json& queryString
        );
};

}
