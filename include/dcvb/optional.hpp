#ifndef DCVB_OPTIONAL_HPP
#define DCVB_OPTIONAL_HPP

#include <dcvb/exception.hpp>
#include <functional>
#include <source_location>
#include <variant>

#if __has_include(<optional>)
#include <optional>
#endif

namespace dcvb {
template <typename T, typename E>
class Result;
template <typename T>
struct Err;
template <typename T>
struct Ok;
}  // namespace dcvb

namespace dcvb {
/**
 * @brief A struct that holds a value for the Optional type.
 * @tparam T The type of the value.
 */
template <typename T>
  requires(!std::is_void_v<T>)
struct Some {
  T value;  // Value contained in the Some variant
};

/**
 * @brief A struct that represents the absence of a value for the Optional type.
 */
struct None {};

/**
 * @brief A type that represents an optional value.
 * @tparam T The type of the value.
 */
template <typename T>
class Optional {
 public:
  using ValueType = T;
  using value_type = T;  // Alias for compatibility

  /**
   * @brief Constructs an Optional with a value (Some).
   * @param someRes The value to store.
   */
  Optional(Some<T>&& someRes)  // NOLINT
      : value_(std::in_place_index<1>, std::move(someRes.value)) {}

  /**
   * @brief Constructs an Optional with a value (Some) from a const reference.
   * @param someRes The value to store.
   */
  Optional(const Some<T>& someRes)
      : value_(std::in_place_index<1>, someRes.value) {}

  /**
   * @brief Constructs an Optional in the None state.
   */
  Optional(None&& /*unused*/)  // NOLINT
      : value_(std::in_place_index<0>, std::monostate{}) {}

  /**
   * @brief Default constructor, creates an Optional in the None state.
   */
  Optional() : value_(std::in_place_index<0>, std::monostate{}) {}

  /**
   * @brief Checks if the Optional contains a value.
   * @return true if it is Some, false if it is None.
   */
  [[nodiscard]] constexpr auto isSome() const noexcept -> bool {
    return value_.index() == 1;
  }

  /**
   * @brief Checks if the Optional does not contain a value.
   * @return true if it is None, false if it is Some.
   */
  [[nodiscard]] constexpr auto isNone() const noexcept -> bool {
    return value_.index() == 0;
  }

  /**
   * @brief Implicit conversion to bool for easy checking of Some state.
   * @return true if it is Some, false if it is None.
   */
  [[nodiscard]] constexpr operator bool() const noexcept { return isSome(); }

  /**
   * @brief Unwraps the Optional to get the value. Throws if it's None.
   * @return The value if it is Some.
   * @throws dcvb::Exception if it is None.
   */
  [[nodiscard]] auto unwrap(
      std::source_location loc = std::source_location::current()) & -> T& {
    if (isNone()) {
      throw Exception("dcvb::Optional - Called unwrap on a None value", loc);
    }
    return std::get<1>(value_);
  }

  /**
   * @brief Unwraps the Optional to get the value. Throws if it's None.
   * @return The value if it is Some.
   * @throws dcvb::Exception if it is None.
   */
  [[nodiscard]] auto unwrap(
      std::source_location loc = std::source_location::current()) && -> T&& {
    if (isNone()) {
      throw Exception("dcvb::Optional - Called unwrap on a None value", loc);
    }
    return std::get<1>(std::move(value_));
  }

  /**
   * @brief Unwraps the Optional to get the value. Throws if it's None.
   * @return The value if it is Some.
   * @throws dcvb::Exception if it is None.
   */
  [[nodiscard]] auto unwrap(
      std::source_location loc =
          std::source_location::current()) const& -> const T& {
    if (isNone()) {
      throw Exception("dcvb::Optional - Called unwrap on a None value", loc);
    }
    return std::get<1>(value_);
  }

  /**
   * @brief Unwraps the Optional to get the value, throwing a custom message if it's None.
   * @param msg The custom message to include in the exception if this is None.
   * @return The value if it is Some.
   * @throws dcvb::Exception with the provided message if it is None.
   */
  [[nodiscard]] auto expect(
      std::string_view msg,
      std::source_location loc = std::source_location::current()) & -> T& {
    if (isNone()) {
      throw Exception(std::string(msg), loc);
    }
    return std::get<1>(value_);
  }

  /**
   * @brief Unwraps the Optional to get the value, throwing a custom message if it's None.
   * @param msg The custom message to include in the exception if this is None.
   * @return The value if it is Some.
   * @throws dcvb::Exception with the provided message if it is None.
   */
  [[nodiscard]] auto expect(
      std::string_view msg,
      std::source_location loc = std::source_location::current()) && -> T&& {
    if (isNone()) {
      throw Exception(std::string(msg), loc);
    }
    return std::get<1>(std::move(value_));
  }

  /**
   * @brief Unwraps the Optional to get the value, throwing a custom message if it's None.
   * @param msg The custom message to include in the exception if this is None.
   * @return The value if it is Some.
   * @throws dcvb::Exception with the provided message if it is None.
   */
  [[nodiscard]] auto expect(
      std::string_view msg,
      std::source_location loc =
          std::source_location::current()) const& -> const T& {
    if (isNone()) {
      throw Exception(std::string(msg), loc);
    }
    return std::get<1>(value_);
  }

  /**
   * @brief Returns the value if it's Some, or a provided default value if it's None.
   * @tparam U The type of the default value, which must be convertible to T.
   * @param defaultValue The value to return if this is None.
   * @return The value or the default value.
   */
  template <typename U>
    requires std::convertible_to<U, T>
  [[nodiscard]] auto valueOr(U&& defaultValue) const& -> T {
    if (isSome()) {
      return std::get<1>(value_);
    }
    return static_cast<T>(std::forward<U>(defaultValue));
  }

  /**
   * @brief Returns the value if it's Some, or a provided default value if it's None.
   * @tparam U The type of the default value, which must be convertible to T.
   * @param defaultValue The value to return if this is None.
   * @return The value or the default value.
   */
  template <typename U>
    requires std::convertible_to<U, T>
  [[nodiscard]] auto valueOr(U&& defaultValue) && -> T {
    if (isSome()) {
      return std::get<1>(std::move(value_));
    }
    return static_cast<T>(std::forward<U>(defaultValue));
  }

  /**
   * @brief Maps the value using the provided function if it's Some.
   * @tparam F The type of the mapping function.
   * @param func The mapping function.
   * @return A new Optional containing the mapped value or None.
   */
  template <typename F>
  [[nodiscard]] auto map(F&& func) const& {
    using U = std::invoke_result_t<F, const T&>;
    static_assert(
        !std::is_void_v<U>,
        "Optional::map cannot return void. Use inspect for side effects.");

    using RetType = Optional<U>;

    if (isSome()) {
      return RetType(
          Some<U>{std::invoke(std::forward<F>(func), std::get<1>(value_))});
    }
    return RetType(None{});
  }

  /**
   * @brief Maps the value using the provided function if it's Some.
   * @tparam F The type of the mapping function.
   * @param func The mapping function.
   * @return A new Optional containing the mapped value or None.
   */
  template <typename F>
  [[nodiscard]] auto map(F&& func) & {
    using U = std::invoke_result_t<F, T&>;
    static_assert(!std::is_void_v<U>, "Optional::map cannot return void.");

    using RetType = Optional<U>;
    if (isSome()) {
      return RetType(
          Some<U>{std::invoke(std::forward<F>(func), std::get<1>(value_))});
    }
    return RetType(None{});
  }

  /**
   * @brief Maps the value using the provided function if it's Some.
   * @tparam F The type of the mapping function.
   * @param func The mapping function.
   * @return A new Optional containing the mapped value or None.
   */
  template <typename F>
  [[nodiscard]] auto map(F&& func) && {
    using U = std::invoke_result_t<F, T&&>;
    static_assert(!std::is_void_v<U>, "Optional::map cannot return void.");

    using RetType = Optional<U>;
    if (isSome()) {
      return RetType(Some<U>{
          std::invoke(std::forward<F>(func), std::get<1>(std::move(value_)))});
    }
    return RetType(None{});
  }

  /**
   * @brief Chains another Optional-returning function to be called if this is Some.
   * @tparam F The type of the chaining function.
   * @param func The chaining function.
   * @return The Result of the function or None.
   */
  template <typename F>
  [[nodiscard]] auto andThen(F&& func) const& {
    using RetType = std::invoke_result_t<F, const T&>;
    if (isSome()) {
      return std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return RetType(None{});
  }

  /**
   * @brief Chains another Optional-returning function to be called if this is Some.
   * @tparam F The type of the chaining function.
   * @param func The chaining function.
   * @return The Result of the function or None.
   */
  template <typename F>
  [[nodiscard]] auto andThen(F&& func) && {
    using RetType = std::invoke_result_t<F, T&&>;
    if (isSome()) {
      return std::invoke(std::forward<F>(func), std::get<1>(std::move(value_)));
    }
    return RetType(None{});
  }

  /**
   * @brief Chains another Optional-returning function to be called if this is Some.
   * @tparam F The type of the chaining function.
   * @param func The chaining function.
   * @return The Result of the function or None.
   */
  template <typename F>
  [[nodiscard]] auto andThen(F&& func) & {
    using RetType = std::invoke_result_t<F, T&>;
    if (isSome()) {
      return std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return RetType(None{});
  }

  /**
   * @brief Flattens an Optional<Optional<T>> into an Optional<T>.
   *        Only available when T is itself an Optional.
   *        If the outer Optional is None, returns None.
   *        If the outer Optional is Some, returns the inner Optional as-is.
   */
  [[nodiscard]] auto flatten() const& -> T
      requires requires {
        typename T::ValueType;
        { T(None{}) };
      } {
    if (isNone()) {
      return T(None{});
    }
    return std::get<1>(value_);
  }

  [[nodiscard]] auto flatten() && -> T
      requires requires {
        typename T::ValueType;
        { T(None{}) };
      } {
    if (isNone()) {
      return T(None{});
    }
    return std::get<1>(std::move(value_));
  }

  /**
   * @brief Applies one of two functions depending on whether this Optional is Some or None.
   *
   * Both functions must return the same type. Use this to collapse an Optional
   * into a single value without checking isSome() manually.
   *
   * @tparam SomeFn Callable invocable with T& (lvalue value).
   * @tparam NoneFn Callable invocable with no arguments.
   * @return The return value of whichever function was invoked.
   */
  template <typename SomeFn, typename NoneFn>
  auto match(SomeFn&& someFn, NoneFn&& noneFn) & {
    static_assert(std::invocable<SomeFn, T&>,
                  "Optional::match: someFn must be invocable with T&");
    static_assert(std::invocable<NoneFn>,
                  "Optional::match: noneFn must be invocable with no arguments");
    static_assert(
        std::same_as<std::invoke_result_t<SomeFn, T&>,
                     std::invoke_result_t<NoneFn>>,
        "Optional::match: someFn and noneFn must return the same type");
    if (isSome()) {
      return std::invoke(std::forward<SomeFn>(someFn), std::get<1>(value_));
    }
    return std::invoke(std::forward<NoneFn>(noneFn));
  }

  /**
   * @brief Applies one of two functions depending on whether this Optional is Some or None.
   * @tparam SomeFn Callable invocable with T&& (rvalue value).
   * @tparam NoneFn Callable invocable with no arguments.
   */
  template <typename SomeFn, typename NoneFn>
  auto match(SomeFn&& someFn, NoneFn&& noneFn) && {
    static_assert(std::invocable<SomeFn, T&&>,
                  "Optional::match: someFn must be invocable with T&&");
    static_assert(std::invocable<NoneFn>,
                  "Optional::match: noneFn must be invocable with no arguments");
    static_assert(
        std::same_as<std::invoke_result_t<SomeFn, T&&>,
                     std::invoke_result_t<NoneFn>>,
        "Optional::match: someFn and noneFn must return the same type");
    if (isSome()) {
      return std::invoke(std::forward<SomeFn>(someFn), std::get<1>(std::move(value_)));
    }
    return std::invoke(std::forward<NoneFn>(noneFn));
  }

  /**
   * @brief Applies one of two functions depending on whether this Optional is Some or None.
   * @tparam SomeFn Callable invocable with const T& (const lvalue value).
   * @tparam NoneFn Callable invocable with no arguments.
   */
  template <typename SomeFn, typename NoneFn>
  auto match(SomeFn&& someFn, NoneFn&& noneFn) const& {
    static_assert(std::invocable<SomeFn, const T&>,
                  "Optional::match: someFn must be invocable with const T&");
    static_assert(std::invocable<NoneFn>,
                  "Optional::match: noneFn must be invocable with no arguments");
    static_assert(
        std::same_as<std::invoke_result_t<SomeFn, const T&>,
                     std::invoke_result_t<NoneFn>>,
        "Optional::match: someFn and noneFn must return the same type");
    if (isSome()) {
      return std::invoke(std::forward<SomeFn>(someFn), std::get<1>(value_));
    }
    return std::invoke(std::forward<NoneFn>(noneFn));
  }

  /**
   * @brief Executes a provided function with the value if it's Some.
   * @tparam F The type of the function.
   * @param func The function to execute.
   * @return A reference to this Optional for chaining.
   */
  template <typename F>
  auto inspect(F&& func) const& -> const Optional& {
    if (isSome()) {
      std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return *this;
  }

  /**
   * @brief Executes a provided function with the value if it's Some.
   * @tparam F The type of the function.
   * @param func The function to execute.
   * @return A reference to this Optional for chaining.
   */
  template <typename F>
  auto inspect(F&& func) & -> Optional& {
    if (isSome()) {
      std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return *this;
  }

  /**
   * @brief Executes a provided function with the value if it's Some.
   * @tparam F The type of the function.
   * @param func The function to execute.
   * @return A reference to this Optional for chaining.
   */
  template <typename F>
  auto inspect(F&& func) && -> Optional&& {
    if (isSome()) {
      std::invoke(std::forward<F>(func), std::get<1>(std::move(value_)));
    }
    return std::move(*this);
  }

#ifdef __cpp_lib_optional
  /**
   * @brief Converts this Optional to a std::optional.
   * @return A std::optional containing the value if it's Some, or std::nullopt.
   */
  [[nodiscard]] auto asStdOptional() const& -> std::optional<T> {
    if (isSome()) {
      return std::optional<T>{std::get<1>(value_)};
    }
    return std::nullopt;
  }

  /**
   * @brief Converts this Optional to a std::optional.
   * @return A std::optional containing the value if it's Some, or std::nullopt.
   */
  [[nodiscard]] auto asStdOptional() && -> std::optional<T> {
    if (isSome()) {
      return std::optional<T>{std::get<1>(std::move(value_))};
    }
    return std::nullopt;
  }
#endif

  /**
   * @brief Converts this Optional to a Result.
   * @tparam E The error type for the Result.
   * @param err The error value to use if this is None.
   * @return A Result containing the value if it's Some, or the provided error.
   */
  template <typename E>
  [[nodiscard]] auto okOr(E&& err) const& -> Result<T, E> {
    if (isSome()) {
      return Result<T, E>(Ok<T>{std::get<1>(value_)});
    }
    return Result<T, E>(Err<E>{std::forward<E>(err)});
  }

  /**
   * @brief Converts this Optional to a Result.
   * @tparam E The error type for the Result.
   * @param err The error value to use if this is None.
   * @return A Result containing the value if it's Some, or the provided error.
   */
  template <typename E>
  [[nodiscard]] auto okOr(E&& err) && -> Result<T, E> {
    if (isSome()) {
      return Result<T, E>(Ok<T>{std::get<1>(std::move(value_))});
    }
    return Result<T, E>(Err<E>{std::forward<E>(err)});
  }

 private:
  std::variant<std::monostate, T>
      value_;  // Variant to hold either None (monostate) or a value
};

/**
 * @brief Helper function to create an Optional containing a value.
 * @tparam T The type of the value.
 * @param value The value to store.
 * @return An Optional containing the value.
 */
template <typename T>
[[nodiscard]] auto some(T&& value) -> Optional<std::decay_t<T>> {
  return Optional<std::decay_t<T>>(
      Some<std::decay_t<T>>{std::forward<T>(value)});
}

/**
 * @brief Helper function to create an empty Optional.
 * @tparam T The type of the value.
 * @return An empty Optional.
 */
template <typename T>
[[nodiscard]] auto none() -> Optional<T> {
  return Optional<T>(None{});
}

}  // namespace dcvb

#endif  // DCVB_OPTIONAL_HPP
