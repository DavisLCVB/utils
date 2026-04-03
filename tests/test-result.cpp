#include <cassert>
#include <dcvb/result.hpp>
#define DCVB_USE_SHORT_MACROS
#include <dcvb/assign_or_return.hpp>
#include <dcvb/format.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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

auto helperSuccess() -> Result<int, std::string> {
  return ok<int, std::string>(100);
}

auto helperFailure() -> Result<int, std::string> {
  return err<int, std::string>("failed");
}

auto functionUsingMacroSuccess() -> Result<int, std::string> {
  DCVB_ASSIGN_OR_RETURN(auto val, helperSuccess());
  return ok<int, std::string>(val * 2);
}

auto functionUsingMacroFailure() -> Result<int, std::string> {
  DCVB_ASSIGN_OR_RETURN(auto val, helperFailure());
  return ok<int, std::string>(val * 2); // Should not be reached
}

auto functionUsingShortMacroSuccess() -> Result<int, std::string> {
  AOR(auto val, helperSuccess());
  return ok<int, std::string>(val * 2);
}

void testAssignOrReturn() {
  auto resSuccess = functionUsingMacroSuccess();
  assert(resSuccess.isOk());
  assert(resSuccess.unwrap() == 200);

  auto resShortSuccess = functionUsingShortMacroSuccess();
  assert(resShortSuccess.isOk());
  assert(resShortSuccess.unwrap() == 200);

  auto resFailure = functionUsingMacroFailure();
  assert(resFailure.isErr());
  assert(resFailure.unwrapErr() == "failed");

  std::cout << "testAssignOrReturn passed\n";
}

void testFormatResult() {
#ifdef DCVB_HAS_STD_FORMAT
  Result<int, std::string> resOk = Ok<int>{42};
  assert(std::format("{}", resOk) == "Ok(42)");

  Result<int, std::string> resErr = Err<std::string>{"error"};
  assert(std::format("{}", resErr) == "Err(error)");

  Result<void, int> resVoidOk = Ok<void>{};
  assert(std::format("{}", resVoidOk) == "Ok()");

  Result<void, Error> resErrorErr = Err<Error>{Error("Something failed")};
  std::string formattedErr = std::format("{:v}", resErrorErr);
  assert(formattedErr.find("Err([Generic:unknown error code: 0] Something failed") != std::string::npos);
  assert(formattedErr.find("test-result.cpp:") != std::string::npos);

  std::cout << "testFormatResult passed\n";
#else
  std::cout << "testFormatResult skipped (std::format not available)\n";
#endif
}

void testOrElse() {
  // orElse on Err invokes recovery function
  Result<int, std::string> errRes = Err<std::string>{"not found"};
  auto recovered = errRes.orElse([](const std::string&) -> Result<int, std::string> {
    return ok<int, std::string>(0);
  });
  assert(recovered.isOk());
  assert(recovered.unwrap() == 0);

  // orElse on Ok does NOT invoke function, propagates Ok
  int called = 0;
  Result<int, std::string> okRes = Ok<int>{42};
  auto kept = okRes.orElse([&called](const std::string&) -> Result<int, std::string> {
    called++;
    return ok<int, std::string>(0);
  });
  assert(kept.isOk());
  assert(kept.unwrap() == 42);
  assert(called == 0);

  // orElse can change the error type
  Result<int, int> numErr = Err<int>{404};
  auto strErr = numErr.orElse([](int code) -> Result<int, std::string> {
    return err<int, std::string>("code: " + std::to_string(code));
  });
  assert(strErr.isErr());
  assert(strErr.unwrapErr() == "code: 404");

  // chaining orElse
  Result<int, std::string> chained = Err<std::string>{"first"};
  auto result = std::move(chained)
    .orElse([](std::string) -> Result<int, std::string> {
      return Err<std::string>{"second"};
    })
    .orElse([](std::string) -> Result<int, std::string> {
      return ok<int, std::string>(99);
    });
  assert(result.isOk());
  assert(result.unwrap() == 99);

  // orElse on Result<void, E>
  Result<void, std::string> voidErr = Err<std::string>{"fail"};
  auto voidRecovered = std::move(voidErr).orElse(
      [](std::string) -> Result<void, std::string> {
        return ok<void, std::string>();
      });
  assert(voidRecovered.isOk());

  Result<void, std::string> voidOk = Ok<void>{};
  int voidCalled = 0;
  auto voidKept = std::move(voidOk).orElse(
      [&voidCalled](std::string) -> Result<void, std::string> {
        voidCalled++;
        return ok<void, std::string>();
      });
  assert(voidKept.isOk());
  assert(voidCalled == 0);

  std::cout << "testOrElse passed\n";
}

auto voidHelperOk() -> Result<void, std::string> {
  return ok<void, std::string>();
}

auto voidHelperErr() -> Result<void, std::string> {
  return err<void, std::string>("void failed");
}

auto functionUsingOrReturnSuccess() -> Result<void, std::string> {
  DCVB_OR_RETURN(voidHelperOk());
  DCVB_OR_RETURN(voidHelperOk());
  return ok<void, std::string>();
}

auto functionUsingOrReturnFailure() -> Result<void, std::string> {
  DCVB_OR_RETURN(voidHelperErr());
  return ok<void, std::string>();  // Should not be reached
}

auto functionUsingOorAlias() -> Result<void, std::string> {
  OOR(voidHelperOk());
  return ok<void, std::string>();
}

auto functionOrReturnWithNonVoid() -> Result<void, std::string> {
  // DCVB_OR_RETURN also works for non-void Result (Ok value discarded)
  DCVB_OR_RETURN(helperSuccess());
  return ok<void, std::string>();
}

void testOrReturn() {
  auto resSuccess = functionUsingOrReturnSuccess();
  assert(resSuccess.isOk());

  auto resFailure = functionUsingOrReturnFailure();
  assert(resFailure.isErr());
  assert(resFailure.unwrapErr() == "void failed");

  auto resAlias = functionUsingOorAlias();
  assert(resAlias.isOk());

  auto resNonVoid = functionOrReturnWithNonVoid();
  assert(resNonVoid.isOk());

  std::cout << "testOrReturn passed\n";
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
    testAssignOrReturn();
    testFormatResult();
    testOrElse();
    testOrReturn();

    std::cout << "All tests passed successfully!\n";
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
