#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>


namespace  apigen {

struct HttpRequest {
    std::string method;
    std::string url;

    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> query;

    std::string contentType;
    std::optional<std::string> body;
};

struct HttpResponse {
    int statusCode = 0;

    std::map<std::string, std::string> headers;

    std::string contentType;
    std::optional<std::string> body;

};


struct HttpTransaction {
    HttpRequest request;
    std::optional<HttpResponse> response;
};


struct HttpDocument {
    std::vector<HttpTransaction> transactions;
};


}
