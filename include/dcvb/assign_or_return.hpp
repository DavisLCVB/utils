#ifndef DCVB_ASSIGN_OR_RETURN_HPP
#define DCVB_ASSIGN_OR_RETURN_HPP

#include <utility>

/**
 * @brief Helper macros for token concatenation.
 */
#define DCVB_MACRO_CONCAT_INNER(x, y) x##y
#define DCVB_MACRO_CONCAT(x, y) DCVB_MACRO_CONCAT_INNER(x, y)

/**
 * @brief Macro to unwrap a Result or return its error early.
 *        Allows for inline variable declarations (e.g., DCVB_ASSIGN_OR_RETURN(auto val, func());)
 *        Works in standard C++17.
 * 
 *        Note: This macro should be used as a statement and followed by a semicolon.
 */
#define DCVB_ASSIGN_OR_RETURN(lhs, rexpr)                               \
  auto DCVB_MACRO_CONCAT(_dcvb_res_, __LINE__) = (rexpr);               \
  if (DCVB_MACRO_CONCAT(_dcvb_res_, __LINE__).isErr()) {                \
    return std::move(DCVB_MACRO_CONCAT(_dcvb_res_, __LINE__)).propagate(); \
  }                                                                     \
  lhs = std::move(DCVB_MACRO_CONCAT(_dcvb_res_, __LINE__)).unwrap()

#ifdef DCVB_USE_SHORT_MACROS
#define AOR DCVB_ASSIGN_OR_RETURN
#endif

/**
 * @brief Macro to propagate a Result error early without assigning a value.
 *        Use for Result<void, E> or when the Ok value is not needed.
 *        The enclosing function must return a compatible Result type.
 *
 *        Note: This macro should be used as a statement and followed by a semicolon.
 */
#define DCVB_OR_RETURN(rexpr)                                                  \
  do {                                                                         \
    auto DCVB_MACRO_CONCAT(_dcvb_res_, __LINE__) = (rexpr);                   \
    if (DCVB_MACRO_CONCAT(_dcvb_res_, __LINE__).isErr()) {                     \
      return std::move(DCVB_MACRO_CONCAT(_dcvb_res_, __LINE__)).propagate();   \
    }                                                                          \
  } while (false)

#ifdef DCVB_USE_SHORT_MACROS
#define OOR DCVB_OR_RETURN
#endif

#endif  // DCVB_ASSIGN_OR_RETURN_HPP
