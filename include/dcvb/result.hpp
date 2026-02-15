#ifndef DCVB_RESULT_HPP
#define DCVB_RESULT_HPP

#include <concepts>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#if __has_include(<expected>) && __cplusplus >= 202302L
#include <expected>
#endif

namespace dcvb {

/**
 * @brief A struct that holds an error value for the Result type.
 * @tparam T The type of the error value.
 */
template <typename T>
struct Err {
  T value;  // Error value
};

/**
 * @brief A struct that holds a success value for the Result type.
 * @tparam T The type of the success value.
 */
template <typename T>
struct Ok {
  T value;  // Success value
};

// Specialization for void type to allow Ok<void> without a value member
template <>
struct Ok<void> {};

/**
 * @brief A type that represents either a success (Ok) or an error (Err).
 * @tparam T The type of the success value.
 * @tparam E The type of the error value.
 */
template <typename T, typename E>
class Result {
 public:
  // Constructor for Ok variant
  Result(Ok<T>&& okRes)  // NOLINT
      : value_(std::in_place_index<0>, std::move(okRes.value)) {}

  // Constructor for Ok variant (const)
  Result(const Ok<T>& okRes) : value_(std::in_place_index<0>, okRes.value) {}

  // Constructor for Err variant
  Result(Err<E>&& errRes)  // NOLINT
      : value_(std::in_place_index<1>, std::move(errRes.value)) {}

  // Constructor for Err variant (const)
  Result(const Err<E>& errRes) : value_(std::in_place_index<1>, errRes.value) {}

  // Deleted default constructor to prevent uninitialized state
  Result() = delete;

  /**
   * @brief Checks if the Result is an Ok variant.
   * @return true if the Result is Ok, false otherwise.
   */
  [[nodiscard]] constexpr auto isOk() const noexcept -> bool {
    return value_.index() == 0;
  }

  /**
   * @brief Checks if the Result is an Err variant.
   * @return true if the Result is Err, false otherwise.
   */
  [[nodiscard]] constexpr auto isErr() const noexcept -> bool {
    return value_.index() == 1;
  }

  // Implicit conversion to bool for easy checking of Ok state
  [[nodiscard]] explicit constexpr operator bool() const noexcept {
    return isOk();
  }

  /**
   * @brief Unwraps the Result to get the success value. Throws if it's an Err.
   * @return The success value if the Result is Ok.
   * @throws std::runtime_error if the Result is an Err.
   */
  [[nodiscard]] auto unwrap() & -> T& {
    if (isErr()) {
      throw std::runtime_error("dcvb::Result - Called unwrap on an Err value");
    }
    return std::get<0>(value_);
  }

  /**
   * @brief Unwraps the Result to get the success value. Throws if it's an Err.
   * @return The success value if the Result is Ok.
   * @throws std::runtime_error if the Result is an Err.
   */
  [[nodiscard]] auto unwrap() && -> T&& {
    if (isErr()) {
      throw std::runtime_error("dcvb::Result - Called unwrap on an Err value");
    }
    return std::get<0>(std::move(value_));
  }

  /**
   * @brief Unwraps the Result to get the success value. Throws if it's an Err.
   * @return The success value if the Result is Ok.
   * @throws std::runtime_error if the Result is an Err.
   */
  [[nodiscard]] auto unwrap() const& -> const T& {
    if (isErr()) {
      throw std::runtime_error("dcvb::Result - Called unwrap on an Err value");
    }
    return std::get<0>(value_);
  }

  /**
   * @brief Unwraps the Result to get the error value. Throws if it's an Ok.
   * @return The error value if the Result is Err.
   * @throws std::runtime_error if the Result is an Ok.
   */
  [[nodiscard]] auto unwrapErr() & -> E {
    if (isOk()) {
      throw std::runtime_error(
          "dcvb::Result - Called unwrapErr on an Ok value");
    }
    return std::get<1>(value_);
  }

  /**
   * @brief Unwraps the Result to get the error value. Throws if it's an Ok.
   * @return The error value if the Result is Err.
   * @throws std::runtime_error if the Result is an Ok.
   */
  [[nodiscard]] auto unwrapErr() && -> E&& {
    if (isOk()) {
      throw std::runtime_error(
          "dcvb::Result - Called unwrapErr on an Ok value");
    }
    return std::get<1>(std::move(value_));
  }

  /**
   * @brief Unwraps the Result to get the error value. Throws if it's an Ok.
   * @return The error value if the Result is Err.
   * @throws std::runtime_error if the Result is an Ok.
   */
  [[nodiscard]] auto unwrapErr() const& -> const E& {
    if (isOk()) {
      throw std::runtime_error(
          "dcvb::Result - Called unwrapErr on an Ok value");
    }
    return std::get<1>(value_);
  }

  /**
   * @brief Maps the success value using the provided function if it's an Ok.
   *        If it's an Err, it propagates the error without calling the function.
   * @tparam F The type of the mapping function.
   * @param func The mapping function to apply to the success value.
   * @return A new Result containing the mapped success value or the original error.
   */
  template <typename F>
  [[nodiscard]] auto map(F&& func) & {
    using U = std::invoke_result_t<F, T&>;
    using RetType = Result<U, E>;
    if (isOk()) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(func), std::get<0>(value_));
        return RetType(Ok<void>{});
      } else {
        return RetType(
            Ok<U>{std::invoke(std::forward<F>(func), std::get<0>(value_))});
      }
    }
    return RetType(Err<E>{std::get<1>(value_)});
  }

  /**
   * @brief Maps the success value using the provided function if it's an Ok.
   *        If it's an Err, it propagates the error without calling the function.
   * @tparam F The type of the mapping function.
   * @param func The mapping function to apply to the success value.
   * @return A new Result containing the mapped success value or the original error.
   */
  template <typename F>
  [[nodiscard]] auto map(F&& func) && {
    using U = std::invoke_result_t<F, T&&>;
    using RetType = Result<U, E>;
    if (isOk()) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(func), std::get<0>(std::move(value_)));
        return RetType(Ok<void>{});
      } else {
        return RetType(Ok<U>{std::invoke(std::forward<F>(func),
                                         std::get<0>(std::move(value_)))});
      }
    }
    return RetType(Err<E>{std::get<1>(std::move(value_))});
  }

  /**
   * @brief Maps the error value using the provided function if it's an Err.
   *        If it's an Ok, it propagates the success value without calling the function.
   * @tparam F The type of the mapping function.
   * @param func The mapping function to apply to the error value.
   * @return A new Result containing the original success value or the mapped error.
   */
  template <typename F>
  [[nodiscard]] auto mapErr(F&& func) & {
    using U = std::invoke_result_t<F, E&>;
    using RetType = Result<T, U>;
    if (isErr()) {
      return RetType(
          Err<U>{std::invoke(std::forward<F>(func), std::get<1>(value_))});
    }
    return RetType(Ok<T>{.value = std::get<0>(value_)});
  }

  /**
   * @brief Maps the error value using the provided function if it's an Err.
   *        If it's an Ok, it propagates the success value without calling the function.
   * @tparam F The type of the mapping function.
   * @param func The mapping function to apply to the error value.
   * @return A new Result containing the original success value or the mapped error.
   */
  template <typename F>
  [[nodiscard]] auto mapErr(F&& func) && {
    using U = std::invoke_result_t<F, E&&>;
    using RetType = Result<T, U>;
    if (isErr()) {
      return RetType(Err<U>{
          std::invoke(std::forward<F>(func), std::get<1>(std::move(value_)))});
    }
    return RetType(Ok<T>{.value = std::get<0>(std::move(value_))});
  }

  /**
   * @brief Chains another Result-returning function to be called if this Result is Ok.
   *        If this Result is an Err, it propagates the error without calling the function.
   * @tparam F The type of the chaining function.
   * @param func The chaining function to apply to the success value.
   * @return The Result returned by the chaining function or a new Result containing the original error.
   */
  template <typename F>
  [[nodiscard]] auto andThen(F&& func) & {
    using RetType = std::invoke_result_t<F, T&>;
    if (isOk()) {
      return std::invoke(std::forward<F>(func), std::get<0>(value_));
    }
    return RetType(Err<E>{std::get<1>(value_)});
  }

  /**
   * @brief Chains another Result-returning function to be called if this Result is Ok.
   *        If this Result is an Err, it propagates the error without calling the function.
   * @tparam F The type of the chaining function.
   * @param func The chaining function to apply to the success value.
   * @return The Result returned by the chaining function or a new Result containing the original error.
   */
  template <typename F>
  [[nodiscard]] auto andThen(F&& func) && {
    using RetType = std::invoke_result_t<F, T&&>;
    if (isOk()) {
      return std::invoke(std::forward<F>(func), std::get<0>(std::move(value_)));
    }
    return RetType(Err<E>{std::get<1>(std::move(value_))});
  }

  /**
   * @brief Returns the success value if it's an Ok, or a provided default value if it's an Err.
   * @tparam U The type of the default value, which must be convertible to T.
   * @param defaultValue The default value to return if this Result is an Err.
   * @return The success value if this Result is Ok, or the default value if it's an Err.
   */
  template <typename U>
    requires std::convertible_to<U, T>
  [[nodiscard]] constexpr auto valueOr(U&& defaultValue) const& {
    if (isOk()) {
      return std::get<0>(value_);
    }
    return static_cast<T>(std::forward<U>(defaultValue));
  }

  /**
   * @brief Returns the success value if it's an Ok, or a provided default value if it's an Err.
   * @tparam U The type of the default value, which must be convertible to T.
   * @param defaultValue The default value to return if this Result is an Err.
   * @return The success value if this Result is Ok, or the default value if it's an Err.
   */
  template <typename U>
    requires std::convertible_to<U, T>
  [[nodiscard]] constexpr auto valueOr(U&& defaultValue) && {
    if (isOk()) {
      return std::move(std::get<0>(value_));
    }
    return static_cast<T>(std::forward<U>(defaultValue));
  }

  /**
   * @brief Executes a provided function with the success value if it's an Ok, without modifying the Result.
   * @tparam F The type of the function to execute.
   * @param func The function to execute with the success value.
   * @return A reference to this Result, allowing for chaining.
   */
  template <typename F>
  constexpr auto inspect(F&& func) & -> Result& {
    if (isOk()) {
      std::invoke(std::forward<F>(func), std::get<0>(value_));
    }
    return *this;
  }

  /**
   * @brief Executes a provided function with the success value if it's an Ok, without modifying the Result.
   * @tparam F The type of the function to execute.
   * @param func The function to execute with the success value.
   * @return A reference to this Result, allowing for chaining.
   */
  template <typename F>
  constexpr auto inspect(F&& func) const& -> const Result& {
    if (isOk()) {
      std::invoke(std::forward<F>(func), std::get<0>(value_));
    }
    return *this;
  }

  /**
   * @brief Executes a provided function with the success value if it's an Ok, without modifying the Result.
   * @tparam F The type of the function to execute.
   * @param func The function to execute with the success value.
   * @return A reference to this Result, allowing for chaining.
   */
  template <typename F>
  constexpr auto inspect(F&& func) && -> Result&& {
    if (isOk()) {
      std::invoke(std::forward<F>(func), std::get<0>(value_));
    }
    return std::move(*this);
  }

  /**  * @brief Executes a provided function with the error value if it's an Err, without modifying the Result.
   * @tparam F The type of the function to execute.
   * @param func The function to execute with the error value.
   * @return A reference to this Result, allowing for chaining.
   */
  template <typename F>
  constexpr auto inspectErr(F&& func) & -> Result& {
    if (isErr()) {
      std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return *this;
  }

  /**
   * @brief Executes a provided function with the error value if it's an Err, without modifying the Result.
   * @tparam F The type of the function to execute.
   * @param func The function to execute with the error value.
   * @return A reference to this Result, allowing for chaining.
   */
  template <typename F>
  constexpr auto inspectErr(F&& func) const& -> const Result& {
    if (isErr()) {
      std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return *this;
  }

  /**
   * @brief Executes a provided function with the error value if it's an Err, without modifying the Result.
   * @tparam F The type of the function to execute.
   * @param func The function to execute with the error value.
   * @return A reference to this Result, allowing for chaining.
   */
  template <typename F>
  constexpr auto inspectErr(F&& func) && -> Result&& {
    if (isErr()) {
      std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return std::move(*this);
  }

  /**
   * @brief Unwraps the Result to get the success value, throwing a custom message if it's an Err.
   * @param msg The custom message to include in the exception if the Result is an Err.
   * @return The success value if the Result is Ok.
   * @throws std::runtime_error with the provided message if the Result is an Err.
   */
  constexpr auto expect(std::string_view msg) & -> T& {
    if (isErr()) {
      throw std::runtime_error(std::string(msg));
    }
    return std::get<0>(value_);
  }

  /**
   * @brief Unwraps the Result to get the success value, throwing a custom message if it's an Err.
   * @param msg The custom message to include in the exception if the Result is an Err.
   * @return The success value if the Result is Ok.
   * @throws std::runtime_error with the provided message if the Result is an Err.
   */
  constexpr auto expect(std::string_view msg) && -> T&& {
    if (isErr()) {
      throw std::runtime_error(std::string(msg));
    }
    return std::move(std::get<0>(value_));
  }

  /**
   * @brief Unwraps the Result to get the success value, throwing a custom message if it's an Err.
   * @param msg The custom message to include in the exception if the Result is an Err.
   * @return The success value if the Result is Ok.
   * @throws std::runtime_error with the provided message if the Result is an Err.
   */
  constexpr auto expect(std::string_view msg) const& -> const T& {
    if (isErr()) {
      throw std::runtime_error(std::string(msg));
    }
    return std::get<0>(value_);
  }

#ifdef __cpp_lib_expected
  /**
   * @brief Converts this Result into a std::expected, where Ok maps to a value and Err maps to an error.
   * @return A std::expected containing the success value if this Result is Ok, or the error value if it's an Err.
   */
  [[nodiscard]] constexpr auto asStdExpected() const& -> std::expected<T, E> {
    if (isOk()) {
      return std::get<0>(value_);
    }
    return std::unexpected(std::get<1>(value_));
  }

  /**
   * @brief Converts this Result into a std::expected, where Ok maps to a value and Err maps to an error.
   * @return A std::expected containing the success value if this Result is Ok, or the error value if it's an Err.
   */
  [[nodiscard]] constexpr auto asStdExpected() && -> std::expected<T, E> {
    if (isOk()) {
      return std::get<0>(std::move(value_));
    }
    return std::unexpected(std::get<1>(std::move(value_)));
  }
#endif

 private:
  std::variant<T, E>
      value_;  // Variant to hold either the success value (Ok) or the error value (Err)
};

/**
 * @brief Specialization of the Result class for the void success type, allowing for operations that only return errors.
 * @tparam E The type of the error value.
 */
template <typename E>
class Result<void, E> {
 public:
  // Constructor for Ok<void> variant
  constexpr Result(Ok<void>&& /*unused*/)  // NOLINT
      : value_(std::in_place_index<0>, std::monostate{}) {}

  // Constructor for Ok<void> variant (const)
  constexpr Result(const Ok<void>& /*unused*/)
      : value_(std::in_place_index<0>, std::monostate{}) {}

  // Constructor for Err variant
  constexpr Result(Err<E>&& errRes)  // NOLINT
      : value_(std::in_place_index<1>, std::move(errRes.value)) {}

  // Constructor for Err variant (const)
  constexpr Result(const Err<E>& errRes)
      : value_(std::in_place_index<1>, errRes.value) {}

  // Deleted default constructor to prevent uninitialized state
  Result() = delete;

  /**
   * @brief Checks if the Result is an Ok variant.
   * @return true if the Result is Ok, false otherwise.
   */
  [[nodiscard]] constexpr auto isOk() const noexcept -> bool {
    return value_.index() == 0;
  }

  /**
   * @brief Checks if the Result is an Err variant.
   * @return true if the Result is Err, false otherwise.
   */
  [[nodiscard]] constexpr auto isErr() const noexcept -> bool {
    return value_.index() == 1;
  }

  // Implicit conversion to bool for easy checking of Ok state
  [[nodiscard]] constexpr operator bool() const noexcept { return isOk(); }

  /**
   * @brief Unwraps the Result to get the success value. Throws if it's an Err.
   * @return void if the Result is Ok.
   * @throws std::runtime_error if the Result is an Err.
   */
  auto unwrap() const -> void {
    if (isErr()) {
      throw std::runtime_error("dcvb::Result - Called unwrap on an Err value");
    }
  }

  /**
   * @brief Unwraps the Result to get the error value. Throws if it's an Ok.
   * @return The error value if the Result is Err.
   * @throws std::runtime_error if the Result is an Ok.
   */
  [[nodiscard]] auto unwrapErr() & -> E& {
    if (isOk()) {
      throw std::runtime_error(
          "dcvb::Result - Called unwrapErr on an Ok value");
    }
    return std::get<1>(value_);
  }

  /**
   * @brief Unwraps the Result to get the error value. Throws if it's an Ok.
   * @return The error value if the Result is Err.
   * @throws std::runtime_error if the Result is an Ok.
   */
  [[nodiscard]] auto unwrapErr() && -> E&& {
    if (isOk()) {
      throw std::runtime_error(
          "dcvb::Result - Called unwrapErr on an Ok value");
    }
    return std::get<1>(std::move(value_));
  }

  /**
   * @brief Unwraps the Result to get the error value. Throws if it's an Ok.
   * @return The error value if the Result is Err.
   * @throws std::runtime_error if the Result is an Ok.
   */
  [[nodiscard]] auto unwrapErr() const& -> const E& {
    if (isOk()) {
      throw std::runtime_error(
          "dcvb::Result - Called unwrapErr on an Ok value");
    }
    return std::get<1>(value_);
  }

  /**
   * @brief Maps the success value using the provided function if it's an Ok.
   *        If it's an Err, it propagates the error without calling the function.
   * @tparam F The type of the mapping function.
   * @param func The mapping function to apply to the success value.
   * @return A new Result containing the mapped success value or the original error.
   */
  template <typename F>
  [[nodiscard]] auto map(F&& func) & {
    using U = std::invoke_result_t<F>;
    using RetType = Result<U, E>;

    if (isOk()) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(func));
        return RetType(Ok<void>{});
      } else {
        return RetType(Ok<U>{std::invoke(std::forward<F>(func))});
      }
    }
    return RetType(Err<E>{std::get<1>(value_)});
  }

  /**
   * @brief Maps the success value using the provided function if it's an Ok.
   *        If it's an Err, it propagates the error without calling the function.
   * @tparam F The type of the mapping function.
   * @param func The mapping function to apply to the success value.
   * @return A new Result containing the mapped success value or the original error.
   */
  template <typename F>
  [[nodiscard]] auto map(F&& func) && {
    using U = std::invoke_result_t<F>;
    using RetType = Result<U, E>;

    if (isOk()) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(func));
        return RetType(Ok<void>{});
      } else {
        return RetType(Ok<U>{std::invoke(std::forward<F>(func))});
      }
    }
    return RetType(Err<E>{std::get<1>(std::move(value_))});
  }

  /**
   * @brief Maps the error value using the provided function if it's an Err.
   *        If it's an Ok, it propagates the success value without calling the function.
   * @tparam F The type of the mapping function.
   * @param func The mapping function to apply to the error value.
   * @return A new Result containing the original success value or the mapped error.
   */
  template <typename F>
  [[nodiscard]] auto mapErr(F&& func) & {
    using U = std::invoke_result_t<F, E&>;
    using RetType = Result<void, U>;
    if (isErr()) {
      return RetType(
          Err<U>{std::invoke(std::forward<F>(func), std::get<1>(value_))});
    }
    return RetType(Ok<void>{});
  }

  /**
   * @brief Maps the error value using the provided function if it's an Err.
   *        If it's an Ok, it propagates the success value without calling the function.
   * @tparam F The type of the mapping function.
   * @param func The mapping function to apply to the error value.
   * @return A new Result containing the original success value or the mapped error.
   */
  template <typename F>
  [[nodiscard]] auto mapErr(F&& func) const& {
    using U = std::invoke_result_t<F, E&>;
    using RetType = Result<void, U>;
    if (isErr()) {
      return RetType(
          Err<U>{std::invoke(std::forward<F>(func), std::get<1>(value_))});
    }
    return RetType(Ok<void>{});
  }

  /**
   * @brief Maps the error value using the provided function if it's an Err.
   *        If it's an Ok, it propagates the success value without calling the function.
   * @tparam F The type of the mapping function.
   * @param func The mapping function to apply to the error value.
   * @return A new Result containing the original success value or the mapped error.
   */
  template <typename F>
  [[nodiscard]] auto mapErr(F&& func) && {
    using U = std::invoke_result_t<F, E&&>;
    using RetType = Result<void, U>;
    if (isErr()) {
      return RetType(Err<U>{
          std::invoke(std::forward<F>(func), std::get<1>(std::move(value_)))});
    }
    return RetType(Ok<void>{});
  }

  /**
   * @brief Chains another Result-returning function to be called if this Result is Ok.
   *        If this Result is an Err, it propagates the error without calling the function.
   * @tparam F The type of the chaining function.
   * @param func The chaining function to apply to the success value.
   * @return The Result returned by the chaining function or a new Result containing the original error.
   */
  template <typename F>
  [[nodiscard]] auto andThen(F&& func) & {
    using RetType = std::invoke_result_t<F>;
    if (isOk()) {
      return std::invoke(std::forward<F>(func));
    }
    return RetType(Err<E>{std::get<1>(value_)});
  }

  /**
   * @brief Chains another Result-returning function to be called if this Result is Ok.
   *        If this Result is an Err, it propagates the error without calling the function.
   * @tparam F The type of the chaining function.
   * @param func The chaining function to apply to the success value.
   * @return The Result returned by the chaining function or a new Result containing the original error.
   */
  template <typename F>
  [[nodiscard]] auto andThen(F&& func) && {
    using RetType = std::invoke_result_t<F>;
    if (isOk()) {
      return std::invoke(std::forward<F>(func));
    }
    return RetType(Err<E>{std::get<1>(std::move(value_))});
  }

  /**
   * @brief Executes a provided function with the success value if it's an Ok, without modifying the Result.
   * @tparam F The type of the function to execute.
   * @param func The function to execute with the success value.
   * @return A reference to this Result, allowing for chaining.
   */
  template <typename F>
  constexpr auto inspect(F&& func) & -> Result& {
    if (isOk()) {
      std::invoke(std::forward<F>(func));
    }
    return *this;
  }

  /**
   * @brief Executes a provided function with the success value if it's an Ok, without modifying the Result.
   * @tparam F The type of the function to execute.
   * @param func The function to execute with the success value.
   * @return A reference to this Result, allowing for chaining.
   */
  template <typename F>
  constexpr auto inspect(F&& func) const& -> const Result& {
    if (isOk()) {
      std::invoke(std::forward<F>(func));
    }
    return *this;
  }

  /**
   * @brief Executes a provided function with the success value if it's an Ok, without modifying the Result.
   * @tparam F The type of the function to execute.
   * @param func The function to execute with the success value.
   * @return A reference to this Result, allowing for chaining.
   */
  template <typename F>
  constexpr auto inspect(F&& func) && -> Result&& {
    if (isOk()) {
      std::invoke(std::forward<F>(func));
    }
    return std::move(*this);
  }

  /**
   * @brief Executes a provided function with the error value if it's an Err, without modifying the Result.
   * @tparam F The type of the function to execute.
   * @param func The function to execute with the error value.
   * @return A reference to this Result, allowing for chaining.
   */
  template <typename F>
  constexpr auto inspectErr(F&& func) & -> Result& {
    if (isErr()) {
      std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return *this;
  }

  /**
   * @brief Executes a provided function with the error value if it's an Err, without modifying the Result.
   * @tparam F The type of the function to execute.
   * @param func The function to execute with the error value.
   * @return A reference to this Result, allowing for chaining.
   */
  template <typename F>
  constexpr auto inspectErr(F&& func) const& -> const Result& {
    if (isErr()) {
      std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return *this;
  }

  /**
   * @brief Executes a provided function with the error value if it's an Err, without modifying the Result.
   * @tparam F The type of the function to execute.
   * @param func The function to execute with the error value.
   * @return A reference to this Result, allowing for chaining.
   */
  template <typename F>
  constexpr auto inspectErr(F&& func) && -> Result&& {
    if (isErr()) {
      std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return std::move(*this);
  }

  /**
   * @brief Unwraps the Result to get the success value, throwing a custom message if it's an Err.
   * @param msg The custom message to include in the exception if the Result is an Err.
   * @return void if the Result is Ok.
   * @throws std::runtime_error with the provided message if the Result is an Err.
   */
  constexpr auto expect(std::string_view msg) const -> void {
    if (isErr()) {
      throw std::runtime_error(std::string(msg));
    }
  }

#ifdef __cpp_lib_expected
  /**
   * @brief Converts this Result into a std::expected, where Ok maps to a value and Err maps to an error.
   * @return A std::expected containing the success value if this Result is Ok, or the error value if it's an Err.
   */
  [[nodiscard]] constexpr auto asStdExpected() const -> std::expected<void, E> {
    if (isOk()) {
      return {};
    }
    return std::unexpected(std::get<1>(value_));
  }
#endif

 private:
  std::variant<std::monostate, E>
      value_;  // Variant to hold either the success state (Ok<void>) or the error value (Err<E>)
};

/**
 * @brief Helper function to create a Result in the Ok state with the provided value.
 * @tparam T The type of the success value.
 * @tparam E The type of the error value.
 * @param value The success value to store in the Result.
 * @return A Result containing the provided success value.
 */
template <typename T, typename E>
[[nodiscard]] constexpr auto ok(T&& value) -> Result<std::decay_t<T>, E> {
  return Result<std::decay_t<T>, E>(
      Ok<std::decay_t<T>>{std::forward<T>(value)});
}

/**
 * @brief Helper function to create a Result in the Ok state for void type.
 * @tparam T The type of the success value (must be void).
 * @tparam E The type of the error value.
 * @return A Result representing a successful operation with no value.
 */
template <typename T, typename E>
  requires std::is_void_v<T>
[[nodiscard]] constexpr auto ok() -> Result<void, E> {
  return Result<void, E>(Ok<void>{});
}

/**
 * @brief Helper function to create a Result in the Err state with the provided error value.
 * @tparam T The type of the success value.
 * @tparam E The type of the error value.
 * @param error The error value to store in the Result.
 * @return A Result containing the provided error value.
 */
template <typename T, typename E>
[[nodiscard]] constexpr auto err(E&& error) -> Result<T, std::decay_t<E>> {
  return Result<T, std::decay_t<E>>(
      Err<std::decay_t<E>>{std::forward<E>(error)});
}

}  // namespace dcvb
#endif  // DCVB_RESULT_HPP
