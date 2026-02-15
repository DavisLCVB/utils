#include <cassert>
#include <dcvb/result.hpp>
#include <iostream>
#include <memory>
#include <string>

using namespace dcvb;

void testSimpleOk() {
  Result<int, std::string> res = Ok<int>{42};
  assert(res.isOk());
  assert(!res.isErr());
  assert(static_cast<bool>(res) == true);
  assert(res.unwrap() == 42);
  std::cout << "testSimpleOk passed\n";
}

void testSimpleErr() {
  Result<int, std::string> res = Err<std::string>{"error"};
  assert(!res.isOk());
  assert(res.isErr());
  assert(static_cast<bool>(res) == false);
  assert(res.unwrapErr() == "error");
  std::cout << "testSimpleErr passed\n";
}

void testVoidResult() {
  Result<void, int> res = Ok<void>{};
  assert(res.isOk());
  res.unwrap();  // Should not throw

  Result<void, int> errRes = Err<int>{500};
  assert(errRes.isErr());
  assert(errRes.unwrapErr() == 500);
  std::cout << "testVoidResult passed\n";
}

void testMapAndThen() {
  auto res = Result<int, int>(Ok<int>{10})
                 .map([](int x) -> int { return x * 2; })  // 20
                 .andThen([](int x) -> Result<std::string, int> {
                   return Ok<std::string>{std::to_string(x)};
                 });

  assert(res.isOk());
  assert(res.unwrap() == "20");

  auto errRes = Result<int, int>(Ok<int>{10})
                    .map([](int x) -> int { return x * 2; })
                    .andThen([](int x) -> Result<std::string, int> {
                      (void)x;
                      return Err<int>{404};
                    });

  assert(errRes.isErr());
  assert(errRes.unwrapErr() == 404);
  std::cout << "testMapAndThen passed\n";
}

void testValueOr() {
  Result<int, std::string> okRes = Ok<int>{42};
  assert(okRes.valueOr(0) == 42);

  Result<int, std::string> errRes = Err<std::string>{"error"};
  assert(errRes.valueOr(0) == 0);
  std::cout << "testValueOr passed\n";
}

void testInspect() {
  int count = 0;
  Result<int, std::string> res = Ok<int>{42};
  res.inspect([&count](int x) -> void {
    assert(x == 42);
    count++;
  });
  assert(count == 1);

  res.inspectErr([&count](const std::string&) -> void { count++; });
  assert(count == 1);  // Should not change
  std::cout << "testInspect passed\n";
}

void testMoveOnlyTypes() {
  Result<std::unique_ptr<int>, int> res =
      Ok<std::unique_ptr<int>>{std::make_unique<int>(42)};
  assert(res.isOk());

  auto ptr = std::move(res).unwrap();
  assert(*ptr == 42);

  Result<int, std::unique_ptr<std::string>> errRes =
      Err<std::unique_ptr<std::string>>{std::make_unique<std::string>("error")};
  assert(errRes.isErr());

  auto errPtr = std::move(errRes).unwrapErr();
  assert(*errPtr == "error");
  std::cout << "testMoveOnlyTypes passed\n";
}

void testEdgeCases() {
  try {
    Result<int, int> res = Err<int>{1};
    [[maybe_unused]] auto ignore = res.unwrap();
    assert(false && "Should have thrown");
  } catch (const std::runtime_error& e) {
    assert(std::string(e.what()).find("unwrap") != std::string::npos);
  }

  try {
    Result<int, int> res = Ok<int>{1};
    [[maybe_unused]] auto ignore = res.unwrapErr();
    assert(false && "Should have thrown");
  } catch (const std::runtime_error& e) {
    assert(std::string(e.what()).find("unwrapErr") != std::string::npos);
  }

  try {
    Result<int, int> res = Err<int>{1};
    res.expect("Custom message");
    assert(false && "Should have thrown");
  } catch (const std::runtime_error& e) {
    assert(std::string(e.what()) == "Custom message");
  }

  int sideEffect = 0;
  auto voidRes =
      Result<int, int>(Ok<int>{10}).map([&](int x) -> void { sideEffect = x; });
  assert(voidRes.isOk());
  assert(sideEffect == 10);

  auto mapErrRes = Result<int, int>(Err<int>{404}).mapErr([](int e) {
    return std::to_string(e);
  });
  assert(mapErrRes.isErr());
  assert(mapErrRes.unwrapErr() == "404");

  std::cout << "testEdgeCases passed\n";
}

void testComplexChaining() {
  struct Data {
    int value;
  };

  auto result =
      Result<Data, std::string>(Ok<Data>{{10}})
          .map([](Data d) -> int { return d.value + 5; })
          .map([](int x) -> std::vector<int> {
            return std::vector<int>{x, x + 1};
          })
          .andThen([](std::vector<int> v) -> Result<int, std::string> {
            if (v.size() > 1) {
              return Ok<int>{v[1]};
            }
            return Err<std::string>{"too small"};
          });

  assert(result.isOk());
  assert(result.unwrap() == 16);
  std::cout << "testComplexChaining passed\n";
}

void testAsExpected() {
#ifdef __cpp_lib_expected
  Result<int, std::string> okRes = Ok<int>{42};
  auto expOk = okRes.asStdExpected();
  assert(expOk.has_value());
  assert(expOk.value() == 42);

  Result<int, std::string> errRes = Err<std::string>{"error"};
  auto expErr = errRes.asStdExpected();
  assert(!expErr.has_value());
  assert(expErr.error() == "error");

  Result<void, int> okVoidRes = Ok<void>{};
  auto expVoidOk = okVoidRes.asStdExpected();
  assert(expVoidOk.has_value());

  Result<void, int> errVoidRes = Err<int>{500};
  auto expVoidErr = errVoidRes.asStdExpected();
  assert(!expVoidErr.has_value());
  assert(expVoidErr.error() == 500);

  std::cout << "testAsExpected passed\n";
#else
  std::cout << "testAsExpected skipped (std::expected not available)\n";
#endif
}

void testHelpers() {
  auto resOk = ok<int, std::string>(42);
  assert(resOk.isOk());
  assert(resOk.unwrap() == 42);

  auto resVoid = ok<void, std::string>();
  assert(resVoid.isOk());

  auto resErr = err<int, std::string>("error");
  assert(resErr.isErr());
  assert(resErr.unwrapErr() == "error");

  std::cout << "testHelpers passed\n";
}

auto main() -> int {
  try {
    testSimpleOk();
    testSimpleErr();
    testVoidResult();
    testMapAndThen();
    testValueOr();
    testInspect();
    testMoveOnlyTypes();
    testEdgeCases();
    testComplexChaining();
    testAsExpected();
    testHelpers();

    std::cout << "All tests passed successfully!\n";
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
