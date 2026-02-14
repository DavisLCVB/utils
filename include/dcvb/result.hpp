#ifndef INCLUDE_DCVB_RESULT_HPP
#define INCLUDE_DCVB_RESULT_HPP

#include <stdexcept>
#include <variant>

template <typename T>
struct Err {
  T value;  // Error value
};

template <typename T>
struct Ok {
  T value;  // Success value
};

template <typename T, typename E>
class Result {
 public:
  Result(Ok<T>&& okRes) : value_(std::move(okRes.value)) {}  // NOLINT

  Result(const Ok<T>& okRes) : value_(okRes.value) {}

  Result(Err<E>&& errRes) : value_(std::move(errRes.value)) {}  // NOLINT

  Result(const Err<E>& errRes) : value_(errRes.value) {}

  [[nodiscard]] auto isOk() const -> bool {
    return std::holds_alternative<T>(value_);
  }

  [[nodiscard]] auto isErr() const -> bool {
    return std::holds_alternative<E>(value_);
  }

  [[nodiscard]] auto unwrap() & -> T& {
    if (isErr()) {
      throw std::runtime_error("dcvb::Result - Called unwrap on an Err value");
    }
    return std::get<T>(value_);
  }

  [[nodiscard]] auto unwrap() && -> T&& {
    if (isErr()) {
      throw std::runtime_error("dcvb::Result - Called unwrap on an Err value");
    }
    return std::move(std::get<T>(value_));
  }

  [[nodiscard]] auto unwrap() const& -> const T& {
    if (isErr()) {
      throw std::runtime_error("dcvb::Result - Called unwrap on an Err value");
    }
    return std::get<T>(value_);
  }

  [[nodiscard]] auto unwrapErr() & -> E {
    if (isOk()) {
      throw std::runtime_error(
          "dcvb::Result - Called unwrapErr on an Ok value");
    }
    return std::get<E>(value_);
  }

  [[nodiscard]] auto unwrapErr() && -> E&& {
    if (isOk()) {
      throw std::runtime_error(
          "dcvb::Result - Called unwrapErr on an Ok value");
    }
    return std::move(std::get<E>(value_));
  }

  [[nodiscard]] auto unwrapErr() const& -> const E& {
    if (isOk()) {
      throw std::runtime_error(
          "dcvb::Result - Called unwrapErr on an Ok value");
    }
    return std::get<E>(value_);
  }

 private:
  std::variant<T, E> value_;
};

#endif  // INCLUDE_DCVB_RESULT_HPP
