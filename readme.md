# Davis's Utils

A lightweight, header-only C++20 utility library providing `Result`, `Optional`, and `Error` types for modern error handling and value management.

## Features

- **`dcvb::Result<T, E>`**: A type-safe way to return either a success value or an error, inspired by Rust's `Result`.
- **`dcvb::Optional<T>`**: A lightweight optional type similar to `std::optional` but with a more functional API (e.g., `map`, `andThen`, `inspect`).
- **`dcvb::Error`**: A rich error type supporting error domains, custom messages, and `std::source_location` for easy debugging.

## Integration

### Using FetchContent

You can easily integrate this library into your project using CMake's `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
    dcvb-utils
    GIT_REPOSITORY https://github.com/DavisLCVB/utils.git
    GIT_TAG v1.1.2
)

FetchContent_MakeAvailable(dcvb-utils)

# Link to your target
target_link_libraries(your_target PRIVATE dcvb::utils)
```

### Manual Installation

To install the library on your system:

```bash
cmake -B build
cmake --build build
sudo cmake --install build
```

Then, use it in your `CMakeLists.txt`:

```cmake
find_package(dcvb-utils REQUIRED)
target_link_libraries(your_target PRIVATE dcvb::utils)
```

## Requirements

- C++20 compatible compiler (uses `std::variant`, `std::source_location`, `std::format`, etc.)
- CMake 3.20 or higher
