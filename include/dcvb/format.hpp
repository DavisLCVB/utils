#ifndef DCVB_FORMAT_HPP
#define DCVB_FORMAT_HPP

#include <dcvb/bitflags.hpp>
#include <dcvb/error.hpp>
#include <dcvb/optional.hpp>
#include <dcvb/result.hpp>
#include <type_traits>

#if __has_include(<format>)
#include <format>
#define DCVB_HAS_STD_FORMAT
#endif

#if __has_include(<fmt/format.h>)
#include <fmt/format.h>
#define DCVB_HAS_FMT_FORMAT
#endif

#define DCVB_IMPL_ERROR_FORMATTER(NAMESPACE)                        \
  template <>                                                       \
  struct NAMESPACE::formatter<dcvb::Error> {                        \
    bool verbose = false;                                           \
    constexpr auto parse(NAMESPACE::format_parse_context& ctx) {    \
      auto it = ctx.begin();                                        \
      if (it != ctx.end() && *it == 'v') {                          \
        verbose = true;                                             \
        ++it;                                                       \
      }                                                             \
      if (it != ctx.end() && *it != '}') {                          \
        throw NAMESPACE::format_error("invalid format");            \
      }                                                             \
      return it;                                                    \
    }                                                               \
    template <typename FormatContext>                               \
    auto format(const dcvb::Error& err, FormatContext& ctx) const { \
      if (verbose) {                                                \
        return NAMESPACE::format_to(                                \
            ctx.out(), "[{}:{}] {} ({}:{})", err.domain().name(),   \
            err.domain().message(err.code()), err.message(),        \
            err.location().file_name(), err.location().line());     \
      } else {                                                      \
        return NAMESPACE::format_to(                                \
            ctx.out(), "[{}:{}] {}", err.domain().name(),           \
            err.domain().message(err.code()), err.message());       \
      }                                                             \
    }                                                               \
  };

#define DCVB_IMPL_OPTIONAL_FORMATTER(NAMESPACE)                           \
  template <typename T>                                                   \
  struct NAMESPACE::formatter<dcvb::Optional<T>> {                        \
    constexpr auto parse(NAMESPACE::format_parse_context& ctx) {          \
      auto it = ctx.begin();                                              \
      if (it != ctx.end() && *it != '}') {                                \
        throw NAMESPACE::format_error("invalid format");                  \
      }                                                                   \
      return it;                                                          \
    }                                                                     \
    template <typename FormatContext>                                     \
    auto format(const dcvb::Optional<T>& opt, FormatContext& ctx) const { \
      if (opt.isSome()) {                                                 \
        return NAMESPACE::format_to(ctx.out(), "Some({})", opt.unwrap()); \
      }                                                                   \
      return NAMESPACE::format_to(ctx.out(), "None");                     \
    }                                                                     \
  };

#define DCVB_IMPL_RESULT_FORMATTER(NAMESPACE)                                \
  template <typename T, typename E>                                          \
  struct NAMESPACE::formatter<dcvb::Result<T, E>> {                          \
    bool verbose = false;                                                    \
    constexpr auto parse(NAMESPACE::format_parse_context& ctx) {             \
      auto it = ctx.begin();                                                 \
      if (it != ctx.end() && *it == 'v') {                                   \
        verbose = true;                                                      \
        ++it;                                                                \
      }                                                                      \
      if (it != ctx.end() && *it != '}') {                                   \
        throw NAMESPACE::format_error("invalid format");                     \
      }                                                                      \
      return it;                                                             \
    }                                                                        \
    template <typename FormatContext>                                        \
    auto format(const dcvb::Result<T, E>& res, FormatContext& ctx) const {   \
      if (res.isOk()) {                                                      \
        if constexpr (std::is_void_v<T>) {                                   \
          return NAMESPACE::format_to(ctx.out(), "Ok()");                    \
        } else {                                                             \
          return NAMESPACE::format_to(ctx.out(), "Ok({})", res.unwrap());    \
        }                                                                    \
      } else {                                                               \
        if constexpr (std::is_same_v<std::remove_cvref_t<E>, dcvb::Error>) { \
          if (verbose) {                                                     \
            return NAMESPACE::format_to(ctx.out(), "Err({:v})",              \
                                        res.unwrapErr());                    \
          }                                                                  \
        }                                                                    \
        return NAMESPACE::format_to(ctx.out(), "Err({})", res.unwrapErr());  \
      }                                                                      \
    }                                                                        \
  };

#define DCVB_IMPL_BITFLAGS_FORMATTER(NAMESPACE)                                \
  template <typename Enum>                                                     \
  struct NAMESPACE::formatter<dcvb::Bitflags<Enum>> {                          \
    bool hex = false;                                                          \
    constexpr auto parse(NAMESPACE::format_parse_context& ctx) {               \
      auto it = ctx.begin();                                                   \
      if (it != ctx.end() && *it == 'x') {                                     \
        hex = true;                                                            \
        ++it;                                                                  \
      }                                                                        \
      if (it != ctx.end() && *it != '}') {                                     \
        throw NAMESPACE::format_error("invalid format");                       \
      }                                                                        \
      return it;                                                               \
    }                                                                          \
    template <typename FormatContext>                                          \
    auto format(const dcvb::Bitflags<Enum>& flags,                             \
                FormatContext& ctx) const {                                    \
      if (hex) {                                                               \
        return NAMESPACE::format_to(ctx.out(), "Bitflags(0x{:x})",             \
                                    flags.raw());                              \
      }                                                                        \
      return NAMESPACE::format_to(ctx.out(), "Bitflags({})", flags.raw());     \
    }                                                                          \
  };

#ifdef DCVB_HAS_STD_FORMAT
DCVB_IMPL_ERROR_FORMATTER(std)
DCVB_IMPL_OPTIONAL_FORMATTER(std)
DCVB_IMPL_RESULT_FORMATTER(std)
DCVB_IMPL_BITFLAGS_FORMATTER(std)
#endif

#ifdef DCVB_HAS_FMT_FORMAT
DCVB_IMPL_ERROR_FORMATTER(fmt)
DCVB_IMPL_OPTIONAL_FORMATTER(fmt)
DCVB_IMPL_RESULT_FORMATTER(fmt)
DCVB_IMPL_BITFLAGS_FORMATTER(fmt)
#endif

#undef DCVB_IMPL_ERROR_FORMATTER
#undef DCVB_IMPL_OPTIONAL_FORMATTER
#undef DCVB_IMPL_RESULT_FORMATTER
#undef DCVB_IMPL_BITFLAGS_FORMATTER

#endif  // DCVB_FORMAT_HPP
