#include <cassert>
#include <dcvb/try_call.hpp>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace dcvb;

void testReturnsOkOnSuccess() {
  auto res = tryCall<int>([] { return 42; });
  assert(res.isOk());
  assert(res.unwrap() == 42);
  std::cout << "testReturnsOkOnSuccess passed\n";
}

void testReturnsOkVoidOnSuccess() {
  auto res = tryCall([] {});
  assert(res.isOk());
  std::cout << "testReturnsOkVoidOnSuccess passed\n";
}

void testCatchesStdException() {
  auto res = tryCall<int>([] -> int { throw std::runtime_error("bad input"); });
  assert(res.isErr());
  assert(res.unwrapErr().message() == "bad input");
  std::cout << "testCatchesStdException passed\n";
}

void testCatchesUnknownException() {
  auto res = tryCall<int>([] -> int { throw 42; });
  assert(res.isErr());
  assert(res.unwrapErr().message() == "unknown exception");
  std::cout << "testCatchesUnknownException passed\n";
}

void testVoidCatchesException() {
  auto res = tryCall([] { throw std::logic_error("invariant"); });
  assert(res.isErr());
  assert(res.unwrapErr().message() == "invariant");
  std::cout << "testVoidCatchesException passed\n";
}

void testErrorInGenericDomain() {
  auto res = tryCall<int>([] -> int { throw std::out_of_range("index"); });
  assert(res.isErr());
  assert(res.unwrapErr().isDomain(GenericDomain::get()));
  std::cout << "testErrorInGenericDomain passed\n";
}

void testMapErrForCustomType() {
  struct MyError { std::string msg; };

  auto res = tryCall<int>([] -> int { throw std::runtime_error("oops"); })
    .mapErr([](Error e) { return MyError{e.message()}; });
  assert(res.isErr());
  assert(res.unwrapErr().msg == "oops");
  std::cout << "testMapErrForCustomType passed\n";
}

auto main() -> int {
  try {
    testReturnsOkOnSuccess();
    testReturnsOkVoidOnSuccess();
    testCatchesStdException();
    testCatchesUnknownException();
    testVoidCatchesException();
    testErrorInGenericDomain();
    testMapErrForCustomType();

    std::cout << "All tests passed successfully!\n";
  } catch (const std::exception& exc) {
    std::cerr << "Test failed with exception: " << exc.what() << "\n";
    return 1;
  }
  return 0;
}
