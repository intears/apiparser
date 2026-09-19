#include <stdexcept>

class UrlParseError : public std::runtime_error {
public:
    explicit UrlParseError(const std::string& message)
        : std::runtime_error(message) {}

};
