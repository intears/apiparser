#pragma once

#include <stdexcept>
#include <string>

namespace apigen {

enum class ErrorCode {
  FileNotFound,
  FileReadFailed,

  InvalidJson,
  InvalidHar,
  MissingHarLog,
  MissingHarEntries,

  UnsupportedFormat,
  GenerationFailed
};

class Error : public std::runtime_error {
public:
  Error(ErrorCode code, const std::string &message)
      : std::runtime_error(message), code_(code) {}

  ErrorCode code() const noexcept { return code_; }

private:
  ErrorCode code_;
};

class FileError : public Error {
public:
  using Error::Error;
};

class ParseError : public Error {
public:
  using Error::Error;
};

class InvalidJsonError : public ParseError {
public:
  using ParseError::ParseError;
};

class InvalidHarError : public ParseError {
public:
  using ParseError::ParseError;
};

class GenerationError : public Error {
public:
  using Error::Error;
};

} // namespace apigen
