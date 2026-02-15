#ifndef DCVB_OPTIONAL_HPP
#define DCVB_OPTIONAL_HPP

#include <functional>
#include <stdexcept>
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
template <typename T>
  requires(!std::is_void_v<T>)
struct Some {
  T value;  // Value contained in the Some variant
};

struct None {};

template <typename T>
class Optional {
 public:
  Optional(Some<T>&& someRes)  // NOLINT
      : value_(std::in_place_index<1>, std::move(someRes.value)) {}

  Optional(const Some<T>& someRes)
      : value_(std::in_place_index<1>, someRes.value) {}

  Optional(None&& /*unused*/)  // NOLINT
      : value_(std::in_place_index<0>, std::monostate{}) {}

  Optional() : value_(std::in_place_index<0>, std::monostate{}) {}

  [[nodiscard]] constexpr auto isSome() const noexcept -> bool {
    return value_.index() == 1;
  }

  [[nodiscard]] constexpr auto isNone() const noexcept -> bool {
    return value_.index() == 0;
  }

  [[nodiscard]] constexpr operator bool() const noexcept { return isSome(); }

  [[nodiscard]] auto unwrap() & -> T& {
    if (isNone()) {
      throw std::runtime_error(
          "dcvb::Optional - Called unwrap on a None value");
    }
    return std::get<1>(value_);
  }

  [[nodiscard]] auto unwrap() && -> T&& {
    if (isNone()) {
      throw std::runtime_error(
          "dcvb::Optional - Called unwrap on a None value");
    }
    return std::get<1>(std::move(value_));
  }

  [[nodiscard]] auto unwrap() const& -> const T& {
    if (isNone()) {
      throw std::runtime_error(
          "dcvb::Optional - Called unwrap on a None value");
    }
    return std::get<1>(value_);
  }

  template <typename U>
    requires std::convertible_to<U, T>
  [[nodiscard]] auto valueOr(U&& defaultValue) const& -> T {
    if (isSome()) {
      return std::get<1>(value_);
    }
    return static_cast<T>(std::forward<U>(defaultValue));
  }

  template <typename U>
    requires std::convertible_to<U, T>
  [[nodiscard]] auto valueOr(U&& defaultValue) && -> T {
    if (isSome()) {
      return std::get<1>(std::move(value_));
    }
    return static_cast<T>(std::forward<U>(defaultValue));
  }

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

  template <typename F>
  [[nodiscard]] auto andThen(F&& func) const& {
    using RetType = std::invoke_result_t<F, const T&>;
    if (isSome()) {
      return std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return RetType(None{});
  }

  template <typename F>
  [[nodiscard]] auto andThen(F&& func) && {
    using RetType = std::invoke_result_t<F, T&&>;
    if (isSome()) {
      return std::invoke(std::forward<F>(func), std::get<1>(std::move(value_)));
    }
    return RetType(None{});
  }

  template <typename F>
  [[nodiscard]] auto andThen(F&& func) & {
    using RetType = std::invoke_result_t<F, T&>;
    if (isSome()) {
      return std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return RetType(None{});
  }

  template <typename F>
  auto inspect(F&& func) const& -> const Optional& {
    if (isSome()) {
      std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return *this;
  }

  template <typename F>
  auto inspect(F&& func) & -> Optional& {
    if (isSome()) {
      std::invoke(std::forward<F>(func), std::get<1>(value_));
    }
    return *this;
  }

  template <typename F>
  auto inspect(F&& func) && -> Optional&& {
    if (isSome()) {
      std::invoke(std::forward<F>(func), std::get<1>(std::get<1>(value_)));
    }
    return std::move(*this);
  }

#ifdef __cpp_lib_optional
  [[nodiscard]] auto asStdOptional() const& -> std::optional<T> {
    if (isSome()) {
      return std::optional<T>{std::get<1>(value_)};
    }
    return std::nullopt;
  }

  [[nodiscard]] auto asStdOptional() && -> std::optional<T> {
    if (isSome()) {
      return std::optional<T>{std::get<1>(std::move(value_))};
    }
    return std::nullopt;
  }
#endif

  template <typename E>
  [[nodiscard]] auto okOr(E&& err) const& -> Result<T, E> {
    if (isSome()) {
      return Result<T, E>(Ok<T>{std::get<1>(value_)});
    }
    return Result<T, E>(Err<E>{std::forward<E>(err)});
  }

  template <typename E>
  [[nodiscard]] auto okOr(E&& err) && -> Result<T, E> {
    if (isSome()) {
      return Result<T, E>(Ok<T>{std::get<1>(std::move(value_))});
    }
    return Result<T, E>(Err<E>{std::forward<E>(err)});
  }

 private:
  std::variant<std::monostate, T> value_;
};

template <typename T>
[[nodiscard]] auto some(T&& value) -> Optional<std::decay_t<T>> {
  return Optional<std::decay_t<T>>(
      Some<std::decay_t<T>>{std::forward<T>(value)});
}

template <typename T>
[[nodiscard]] auto none() -> Optional<T> {
  return Optional<T>(None{});
}

}  // namespace dcvb

#endif  // DCVB_OPTIONAL_HPP
