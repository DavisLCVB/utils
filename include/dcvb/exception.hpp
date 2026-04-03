#ifndef DCVB_EXCEPTION_HPP
#define DCVB_EXCEPTION_HPP

#include <concepts>
#include <source_location>
#include <stdexcept>
#include <string>

namespace dcvb {

namespace detail {
/**
 * @brief Concept satisfied by types that expose a toString() method returning
 *        something convertible to std::string.
 */
template <typename T>
concept HasToString = requires(const T& obj) {
  { obj.toString() } -> std::convertible_to<std::string>;
};
}  // namespace detail

/**
 * @brief Exception type for unrecoverable errors (analogous to Rust's panic!).
 *
 * dcvb::Exception is thrown when the program encounters a state that should
 * never occur: unwrapping a None/Err value, violating an invariant, or any
 * other condition that indicates a programming error rather than a recoverable
 * runtime failure.
 *
 * Extends std::runtime_error so existing catch(std::runtime_error&) and
 * catch(std::exception&) blocks remain compatible.
 */
class Exception : public std::runtime_error {
 public:
  /**
   * @brief Constructs an Exception with a message and source location.
   * @param msg The message describing the unrecoverable condition.
   * @param loc The source location where the exception was triggered (captured automatically).
   */
  explicit Exception(const std::string& msg,
                     std::source_location loc = std::source_location::current())
      : std::runtime_error(msg), location_(loc) {}

  /**
   * @brief Returns the source location where the exception was triggered.
   * @return A reference to the captured std::source_location.
   */
  [[nodiscard]] auto location() const noexcept -> const std::source_location& {
    return location_;
  }

 private:
  std::source_location location_;
};

/**
 * @brief Raises a dcvb::Exception with the provided message. Never returns.
 *
 * Use this for invariant violations and unrecoverable states that indicate
 * a programming error, not a recoverable runtime condition.
 *
 * @param msg The exception message.
 * @param loc The source location (captured automatically at the call site).
 */
[[noreturn]] inline auto raise(
    const std::string& msg,
    std::source_location loc = std::source_location::current()) -> void {
  throw Exception(msg, loc);
}

/**
 * @brief Raises a dcvb::Exception from any error type that exposes toString().
 *
 * Works with dcvb::Error and any custom error type with a toString() method,
 * without requiring a cross-header dependency or conditional compilation.
 *
 * @tparam E An error type satisfying detail::HasToString.
 * @param error The error to raise.
 * @param loc   The source location (captured automatically at the call site).
 */
template <detail::HasToString E>
[[noreturn]] inline auto raise(
    const E& error,
    std::source_location loc = std::source_location::current()) -> void {
  throw Exception(error.toString(), loc);
}

}  // namespace dcvb

#endif  // DCVB_EXCEPTION_HPP
