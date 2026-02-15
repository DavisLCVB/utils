#ifndef DCVB_ERROR_HPP
#define DCVB_ERROR_HPP

#include <format>
#include <ostream>
#include <source_location>
#include <string>
#include <utility>

namespace dcvb {
template <typename T>
struct Err;

template <typename T, typename E>
class Result;
}  // namespace dcvb

namespace dcvb {
struct ErrorDomain {
  ErrorDomain() = default;
  ErrorDomain(const ErrorDomain&) = default;
  ErrorDomain(ErrorDomain&&) = default;
  auto operator=(const ErrorDomain&) -> ErrorDomain& = default;
  auto operator=(ErrorDomain&&) -> ErrorDomain& = default;
  virtual ~ErrorDomain() = default;

  [[nodiscard]] virtual auto name() const -> const char* = 0;

  [[nodiscard]] virtual auto message(int code) const -> std::string {
    return "unknown error code: " + std::to_string(code);
  }

  auto operator==(const ErrorDomain& other) const -> bool {
    return this == &other;
  }

  auto operator!=(const ErrorDomain& other) const -> bool {
    return !(*this == other);
  };
};

class GenericDomain : public ErrorDomain {
 public:
  [[nodiscard]] auto name() const -> const char* override { return "Generic"; }

  static auto get() -> const GenericDomain& {
    static GenericDomain instance;
    return instance;
  }

 private:
  GenericDomain() = default;
};

class Error {
 public:
  Error(const ErrorDomain& domain, int code, std::string msg,
        std::source_location loc = std::source_location::current())
      : domain_(&domain),
        code_(code),
        message_(std::move(msg)),
        location_(loc) {}

  Error(std::string msg,
        std::source_location loc = std::source_location::current())
      : Error(GenericDomain::get(), 0, std::move(msg), loc) {}

  [[nodiscard]] auto domain() const -> const ErrorDomain& { return *domain_; }

  [[nodiscard]] auto code() const -> int { return code_; }

  [[nodiscard]] auto message() const -> const std::string& { return message_; }

  [[nodiscard]] auto location() const -> const std::source_location& {
    return location_;
  }

  [[nodiscard]] auto isDomain(const ErrorDomain& domain) const -> bool {
    return *domain_ == domain;
  }

  [[nodiscard]] auto is(const ErrorDomain& domain, int code) const -> bool {
    return *domain_ == domain && code_ == code;
  }

  [[nodiscard]] auto toString() const -> std::string {
#ifdef DCVB_ENABLE_ERROR_LOCATION
    return std::format("[{}:{}] {} ({}:{})", domain_->name(),
                       domain_->message(code_), message_, location_.file_name(),
                       location_.line());
#else
    return std::format("[{}:{}] {}", domain_->name(), domain_->message(code_),
                       message_);
#endif
  }

  friend auto operator<<(std::ostream& os, const Error& error)  // NOLINT
      -> std::ostream& {
    return os << error.toString();
  }

 private:
  const ErrorDomain* domain_;
  int code_;
  std::string message_;
  std::source_location location_;
};
}  // namespace dcvb

#if __has_include(<dcvb/result.hpp>)

#include <dcvb/result.hpp>

namespace dcvb {
template <typename T = void>
[[nodiscard]] auto err(
    const ErrorDomain& domain, int errorCode, std::string msg,
    std::source_location loc = std::source_location::current())
    -> Result<T, Error> {
  return Result<T, Error>(
      Err<Error>{Error(domain, errorCode, std::move(msg), loc)});
}

template <typename T = void>
[[nodiscard]] auto err(
    std::string msg, std::source_location loc = std::source_location::current())
    -> Result<T, Error> {
  return Result<T, Error>(Err<Error>{Error(std::move(msg), loc)});
}
}  // namespace dcvb
#endif

#endif  // INCLUDE_DCVB_ERROR_HPP
