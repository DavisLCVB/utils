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
/**
 * @brief Base class for error domains, which categorize error codes.
 */
struct ErrorDomain {
  ErrorDomain() = default;
  ErrorDomain(const ErrorDomain&) = default;
  ErrorDomain(ErrorDomain&&) = default;
  auto operator=(const ErrorDomain&) -> ErrorDomain& = default;
  auto operator=(ErrorDomain&&) -> ErrorDomain& = default;
  virtual ~ErrorDomain() = default;

  /**
   * @brief Returns the name of the error domain.
   * @return A string representing the domain name.
   */
  [[nodiscard]] virtual auto name() const -> const char* = 0;

  /**
   * @brief Returns a descriptive message for a given error code in this domain.
   * @param code The error code.
   * @return A string describing the error code.
   */
  [[nodiscard]] virtual auto message(int code) const -> std::string {
    return "unknown error code: " + std::to_string(code);
  }

  /**
   * @brief Compares this domain with another for equality.
   * @param other The other error domain to compare against.
   * @return true if the domains are the same, false otherwise.
   */
  auto operator==(const ErrorDomain& other) const -> bool {
    return this == &other;
  }

  /**
   * @brief Compares this domain with another for inequality.
   * @param other The other error domain to compare against.
   * @return true if the domains are different, false otherwise.
   */
  auto operator!=(const ErrorDomain& other) const -> bool {
    return !(*this == other);
  };
};

/**
 * @brief A generic error domain for common errors.
 */
class GenericDomain : public ErrorDomain {
 public:
  /**
   * @brief Returns the name of the generic domain.
   * @return "Generic"
   */
  [[nodiscard]] auto name() const -> const char* override { return "Generic"; }

  /**
   * @brief Returns the singleton instance of the GenericDomain.
   * @return A reference to the static GenericDomain instance.
   */
  static auto get() -> const GenericDomain& {
    static GenericDomain instance;
    return instance;
  }

 private:
  GenericDomain() = default;
};

/**
 * @brief Represents a detailed error with domain, code, message, and source location.
 */
class Error {
 public:
  /**
   * @brief Constructs an Error with a specific domain, code, message, and location.
   * @param domain The error domain.
   * @param code The error code.
   * @param msg The error message.
   * @param loc The source location where the error occurred.
   */
  Error(const ErrorDomain& domain, int code, std::string msg,
        std::source_location loc = std::source_location::current())
      : domain_(&domain),
        code_(code),
        message_(std::move(msg)),
        location_(loc) {}

  /**
   * @brief Constructs an Error in the Generic domain with a message and location.
   * @param msg The error message.
   * @param loc The source location where the error occurred.
   */
  Error(std::string msg,
        std::source_location loc = std::source_location::current())
      : Error(GenericDomain::get(), 0, std::move(msg), loc) {}

  /**
   * @brief Returns the error domain.
   * @return The domain of the error.
   */
  [[nodiscard]] auto domain() const -> const ErrorDomain& { return *domain_; }

  /**
   * @brief Returns the error code.
   * @return The integer error code.
   */
  [[nodiscard]] auto code() const -> int { return code_; }

  /**
   * @brief Returns the error message.
   * @return The descriptive message for the error.
   */
  [[nodiscard]] auto message() const -> const std::string& { return message_; }

  /**
   * @brief Returns the source location where the error was created.
   * @return The std::source_location of the error.
   */
  [[nodiscard]] auto location() const -> const std::source_location& {
    return location_;
  }

  /**
   * @brief Checks if the error belongs to a specific domain.
   * @param domain The domain to check against.
   * @return true if the error is in the given domain, false otherwise.
   */
  [[nodiscard]] auto isDomain(const ErrorDomain& domain) const -> bool {
    return *domain_ == domain;
  }

  /**
   * @brief Checks if the error has a specific domain and code.
   * @param domain The domain to check.
   * @param code The code to check.
   * @return true if both domain and code match, false otherwise.
   */
  [[nodiscard]] auto is(const ErrorDomain& domain, int code) const -> bool {
    return *domain_ == domain && code_ == code;
  }

  /**
   * @brief Returns a string representation of the error.
   * @return A formatted string containing domain, message, and optionally location.
   */
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

  /**
   * @brief Stream operator for easy printing of Error objects.
   * @param os The output stream.
   * @param error The Error object to print.
   * @return The output stream.
   */
  friend auto operator<<(std::ostream& os, const Error& error)  // NOLINT
      -> std::ostream& {
    return os << error.toString();
  }

 private:
  const ErrorDomain* domain_;      // Pointer to the error domain
  int code_;                       // Error code
  std::string message_;            // Error message
  std::source_location location_;  // Source location where error occurred
};
}  // namespace dcvb

#if __has_include(<dcvb/result.hpp>)

#include <dcvb/result.hpp>

namespace dcvb {
/**
 * @brief Helper function to create a Result in the Err state with a detailed Error.
 * @tparam T The success type of the Result.
 * @param domain The error domain.
 * @param errorCode The error code.
 * @param msg The error message.
 * @param loc The source location.
 * @return A Result containing the Error.
 */
template <typename T = void>
[[nodiscard]] auto err(
    const ErrorDomain& domain, int errorCode, std::string msg,
    std::source_location loc = std::source_location::current())
    -> Result<T, Error> {
  return Result<T, Error>(
      Err<Error>{Error(domain, errorCode, std::move(msg), loc)});
}

/**
 * @brief Helper function to create a Result in the Err state with a generic Error.
 * @tparam T The success type of the Result.
 * @param msg The error message.
 * @param loc The source location.
 * @return A Result containing the Error.
 */
template <typename T = void>
[[nodiscard]] auto err(
    std::string msg, std::source_location loc = std::source_location::current())
    -> Result<T, Error> {
  return Result<T, Error>(Err<Error>{Error(std::move(msg), loc)});
}
}  // namespace dcvb
#endif

#endif  // INCLUDE_DCVB_ERROR_HPP
