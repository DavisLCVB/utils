#ifndef DCVB_BITFLAGS_HPP
#define DCVB_BITFLAGS_HPP

#include <concepts>
#include <type_traits>

namespace dcvb {

/**
 * @brief A type-safe wrapper around an enum class that enables bitwise flag operations.
 *
 * Usage:
 * @code
 *   enum class Perms : uint8_t { Read = 1, Write = 2, Exec = 4 };
 *
 *   auto p = Bitflags<Perms>(Perms::Read) | Perms::Write;
 *   p.has(Perms::Read);   // true
 *   p.has(Perms::Exec);   // false
 *   p.set(Perms::Exec);
 *   p.unset(Perms::Write);
 * @endcode
 *
 * @tparam Enum An enum class type to wrap. Its underlying type must be an integer.
 */
template <typename Enum>
  requires(std::is_enum_v<Enum> &&
           std::is_integral_v<std::underlying_type_t<Enum>>)
class Bitflags {
 public:
  using EnumType = Enum;
  using Underlying = std::underlying_type_t<Enum>;

  /**
   * @brief Constructs an empty Bitflags with no bits set.
   */
  constexpr Bitflags() noexcept : value_(0) {}

  /**
   * @brief Constructs a Bitflags from a single enum value.
   * @param flag The initial flag to set.
   */
  constexpr Bitflags(Enum flag) noexcept  // NOLINT(google-explicit-constructor)
      : value_(static_cast<Underlying>(flag)) {}

  /**
   * @brief Constructs a Bitflags directly from a raw underlying integer value.
   * @param raw The raw bitmask value.
   */
  constexpr static auto fromRaw(Underlying raw) noexcept -> Bitflags {
    Bitflags result;
    result.value_ = raw;
    return result;
  }

  [[nodiscard]] constexpr auto operator|(Bitflags other) const noexcept -> Bitflags {
    return fromRaw(value_ | other.value_);
  }

  [[nodiscard]] constexpr auto operator&(Bitflags other) const noexcept -> Bitflags {
    return fromRaw(value_ & other.value_);
  }

  [[nodiscard]] constexpr auto operator^(Bitflags other) const noexcept -> Bitflags {
    return fromRaw(value_ ^ other.value_);
  }

  [[nodiscard]] constexpr auto operator~() const noexcept -> Bitflags {
    return fromRaw(~value_);
  }

  constexpr auto operator|=(Bitflags other) noexcept -> Bitflags& {
    value_ |= other.value_;
    return *this;
  }

  constexpr auto operator&=(Bitflags other) noexcept -> Bitflags& {
    value_ &= other.value_;
    return *this;
  }

  constexpr auto operator^=(Bitflags other) noexcept -> Bitflags& {
    value_ ^= other.value_;
    return *this;
  }

  [[nodiscard]] constexpr auto operator==(const Bitflags& other) const noexcept -> bool = default;

  /**
   * @brief Returns true if ALL bits in @p flags are set in this Bitflags.
   * @param flags The flags to check.
   */
  [[nodiscard]] constexpr auto has(Bitflags flags) const noexcept -> bool {
    return (value_ & flags.value_) == flags.value_;
  }

  /**
   * @brief Returns true if ANY of the bits in @p flags are set in this Bitflags.
   * @param flags The flags to check.
   */
  [[nodiscard]] constexpr auto hasAny(Bitflags flags) const noexcept -> bool {
    return (value_ & flags.value_) != 0;
  }

  /**
   * @brief Returns true if at least one bit is set.
   */
  [[nodiscard]] constexpr auto any() const noexcept -> bool { return value_ != 0; }

  /**
   * @brief Returns true if no bits are set.
   */
  [[nodiscard]] constexpr auto none() const noexcept -> bool { return value_ == 0; }

  /**
   * @brief Explicit conversion to bool. True if any bit is set.
   */
  [[nodiscard]] constexpr explicit operator bool() const noexcept { return any(); }

  /**
   * @brief Sets all bits in @p flags.
   */
  constexpr auto set(Bitflags flags) noexcept -> Bitflags& {
    value_ |= flags.value_;
    return *this;
  }

  /**
   * @brief Clears all bits in @p flags.
   */
  constexpr auto unset(Bitflags flags) noexcept -> Bitflags& {
    value_ &= ~flags.value_;
    return *this;
  }

  /**
   * @brief Toggles all bits in @p flags.
   */
  constexpr auto toggle(Bitflags flags) noexcept -> Bitflags& {
    value_ ^= flags.value_;
    return *this;
  }

  /**
   * @brief Returns the raw underlying integer value.
   */
  [[nodiscard]] constexpr auto raw() const noexcept -> Underlying { return value_; }

 private:
  Underlying value_;
};

/**
 * @brief Combines two enum values into a Bitflags without explicit construction.
 *        Allows writing: Perms::Read | Perms::Write
 */
template <typename Enum>
  requires std::is_enum_v<Enum>
[[nodiscard]] constexpr auto operator|(Enum lhs, Enum rhs) noexcept -> Bitflags<Enum> {
  return Bitflags<Enum>(lhs) | Bitflags<Enum>(rhs);
}

}  // namespace dcvb

#endif  // DCVB_BITFLAGS_HPP
