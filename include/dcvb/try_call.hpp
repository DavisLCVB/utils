#ifndef DCVB_TRY_CALL_HPP
#define DCVB_TRY_CALL_HPP

#include <concepts>
#include <dcvb/error.hpp>
#include <dcvb/result.hpp>
#include <exception>
#include <source_location>

namespace dcvb {

/**
 * @brief Calls @p func and wraps the result in a Result, catching any thrown exception.
 *
 * On success, returns Ok with the value returned by @p func (or Ok<void> if T is void).
 * On std::exception, returns Err containing a dcvb::Error with the exception message.
 * On any other throw, returns Err with the message "unknown exception".
 *
 * The source location captured in the resulting Error points to the tryCall call site.
 * Use mapErr() to convert the Error to a domain-specific error type if needed.
 *
 * @tparam T The expected return type of @p func. Defaults to void.
 * @param func A callable with no parameters returning T.
 * @param loc  Source location captured automatically at the call site.
 * @return Result<T, Error>
 */
template <typename T = void, std::invocable F>
[[nodiscard]] auto tryCall(
    F&& func,
    std::source_location loc = std::source_location::current()) -> Result<T, Error> {
  try {
    if constexpr (std::is_void_v<T>) {
      std::invoke(std::forward<F>(func));
      return ok<void, Error>();
    } else {
      return ok<T, Error>(std::invoke(std::forward<F>(func)));
    }
  } catch (const std::exception& exc) {
    return Result<T, Error>(Err<Error>{Error(exc.what(), loc)});
  } catch (...) {
    return Result<T, Error>(Err<Error>{Error("unknown exception", loc)});
  }
}

}  // namespace dcvb

#endif  // DCVB_TRY_CALL_HPP
