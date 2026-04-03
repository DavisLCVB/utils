#include <cassert>
#include <dcvb/optional.hpp>
#include <dcvb/result.hpp>
#include <dcvb/format.hpp>
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

void testFormatOptional() {
#ifdef DCVB_HAS_STD_FORMAT
  Optional<int> optSome = some(42);
  assert(std::format("{}", optSome) == "Some(42)");

  Optional<int> optNone = none<int>();
  assert(std::format("{}", optNone) == "None");

  std::cout << "testFormatOptional passed\n";
#else
  std::cout << "testFormatOptional skipped (std::format not available)\n";
#endif
}

void testFlattenOptional() {
  // Some(Some(x)) → Some(x)
  auto nested = some(some(42));
  auto flat = nested.flatten();
  assert(flat.isSome());
  assert(flat.unwrap() == 42);

  // Some(None) → None
  auto someNone = some(none<int>());
  auto flatNone = someNone.flatten();
  assert(flatNone.isNone());

  // None → None
  auto outerNone = none<Optional<int>>();
  auto flatOuterNone = outerNone.flatten();
  assert(flatOuterNone.isNone());

  // Works with move (rvalue overload)
  auto flatMoved = some(some(99)).flatten();
  assert(flatMoved.isSome());
  assert(flatMoved.unwrap() == 99);

  std::cout << "testFlattenOptional passed" << std::endl;
}

void testFlattenResult() {
  // Ok(Ok(x)) → Ok(x)
  using R = Result<int, std::string>;
  Result<R, std::string> outer = Ok<R>{ok<int, std::string>(42)};
  auto flat = outer.flatten();
  assert(flat.isOk());
  assert(flat.unwrap() == 42);

  // Ok(Err(e)) → Err(e)
  Result<R, std::string> outerOkInnerErr = Ok<R>{err<int, std::string>("inner error")};
  auto flatInnerErr = outerOkInnerErr.flatten();
  assert(flatInnerErr.isErr());
  assert(flatInnerErr.unwrapErr() == "inner error");

  // Err(e) → Err(e)  (outer error propagates)
  Result<R, std::string> outerErr = err<R, std::string>("outer error");
  auto flatOuterErr = std::move(outerErr).flatten();
  assert(flatOuterErr.isErr());
  assert(flatOuterErr.unwrapErr() == "outer error");

  // Equivalent to andThen vs map + flatten
  auto resMap = ok<int, std::string>(10)
    .map([](int x) { return ok<int, std::string>(x * 2); })
    .flatten();
  auto resAndThen = ok<int, std::string>(10)
    .andThen([](int x) { return ok<int, std::string>(x * 2); });
  assert(resMap.isOk());
  assert(resAndThen.isOk());
  assert(resMap.unwrap() == resAndThen.unwrap());

  std::cout << "testFlattenResult passed" << std::endl;
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
    testFormatOptional();
    testFlattenOptional();
    testFlattenResult();

    std::cout << "All tests passed successfully!" << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

