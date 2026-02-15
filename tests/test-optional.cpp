#include <cassert>
#include <dcvb/optional.hpp>
#include <dcvb/result.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace dcvb;

void testSimpleSome() {
  auto opt = some(42);
  assert(opt.isSome());
  assert(!opt.isNone());
  assert(static_cast<bool>(opt) == true);
  assert(opt.unwrap() == 42);
  std::cout << "testSimpleSome passed" << std::endl;
}

void testSimpleNone() {
  auto opt = none<int>();
  assert(!opt.isSome());
  assert(opt.isNone());
  assert(static_cast<bool>(opt) == false);
  std::cout << "testSimpleNone passed" << std::endl;
}

void testMapAndThen() {
  auto opt = some(10)
                 .map([](int x) { return x * 2; }) // 20
                 .andThen([](int x) { return some(std::to_string(x)); });

  assert(opt.isSome());
  assert(opt.unwrap() == "20");

  auto noneOpt = none<int>()
                     .map([](int x) { return x * 2; })
                     .andThen([](int x) { return some(std::to_string(x)); });

  assert(noneOpt.isNone());
  std::cout << "testMapAndThen passed" << std::endl;
}

void testValueOr() {
  auto optSome = some(42);
  assert(optSome.valueOr(0) == 42);

  auto optNone = none<int>();
  assert(optNone.valueOr(0) == 0);
  std::cout << "testValueOr passed" << std::endl;
}

void testInspect() {
  int count = 0;
  auto opt = some(42);
  opt.inspect([&count](int x) {
    assert(x == 42);
    count++;
  });
  assert(count == 1);

  auto optNone = none<int>();
  optNone.inspect([&count](int) { count++; });
  assert(count == 1); // Should not change
  std::cout << "testInspect passed" << std::endl;
}

void testMoveOnlyTypes() {
  auto opt = some(std::make_unique<int>(42));
  assert(opt.isSome());

  auto ptr = std::move(opt).unwrap();
  assert(*ptr == 42);

  auto optNone = none<std::unique_ptr<int>>();
  assert(optNone.isNone());
  std::cout << "testMoveOnlyTypes passed" << std::endl;
}

void testEdgeCases() {
  try {
    auto opt = none<int>();
    [[maybe_unused]] auto ignore = opt.unwrap();
    assert(false && "Should have thrown");
  } catch (const std::runtime_error& e) {
    assert(std::string(e.what()).find("unwrap") != std::string::npos);
  }

  std::cout << "testEdgeCases passed" << std::endl;
}

void testAsStdOptional() {
#ifdef __cpp_lib_optional
  auto optSome = some(42);
  auto stdOptSome = optSome.asStdOptional();
  assert(stdOptSome.has_value());
  assert(*stdOptSome == 42);

  auto optNone = none<int>();
  auto stdOptNone = optNone.asStdOptional();
  assert(!stdOptNone.has_value());
  std::cout << "testAsStdOptional passed" << std::endl;
#else
  std::cout << "testAsStdOptional skipped" << std::endl;
#endif
}

void testOkOr() {
  auto optSome = some(42);
  auto resOk = optSome.okOr(std::string("error"));
  assert(resOk.isOk());
  assert(resOk.unwrap() == 42);

  auto optNone = none<int>();
  auto resErr = optNone.okOr(std::string("error"));
  assert(resErr.isErr());
  assert(resErr.unwrapErr() == "error");
  std::cout << "testOkOr passed" << std::endl;
}

auto main() -> int {
  try {
    testSimpleSome();
    testSimpleNone();
    testMapAndThen();
    testValueOr();
    testInspect();
    testMoveOnlyTypes();
    testEdgeCases();
    testAsStdOptional();
    testOkOr();

    std::cout << "All tests passed successfully!" << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
