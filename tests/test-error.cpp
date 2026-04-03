#include <cassert>
#include <dcvb/error.hpp>
#include <dcvb/format.hpp>
#include <iostream>
#include <sstream>
#include <string>

using namespace dcvb;

void testGenericDomain() {
  const auto& domain = GenericDomain::get();
  assert(std::string(domain.name()) == "Generic");
  assert(domain.message(0) == "unknown error code: 0");
  assert(domain.message(42) == "unknown error code: 42");
  std::cout << "testGenericDomain passed" << '\n';
}

struct CustomDomain : public ErrorDomain {
  [[nodiscard]] auto name() const -> const char* override { return "Custom"; }

  [[nodiscard]] auto message(int code) const -> std::string override {
    if (code == 1) {
      return "First error";
    }
    return ErrorDomain::message(code);
  }
};

void testCustomDomain() {
  CustomDomain domain;
  assert(std::string(domain.name()) == "Custom");
  assert(domain.message(1) == "First error");
  assert(domain.message(2) == "unknown error code: 2");

  CustomDomain other;
  assert(domain == domain);
  assert(domain != other);
  std::cout << "testCustomDomain passed" << '\n';
}

void testErrorBasic() {
  Error err("Something went wrong");
  assert(err.domain() == GenericDomain::get());
  assert(err.code() == 0);
  assert(err.message() == "Something went wrong");
  assert(err.isDomain(GenericDomain::get()));
  assert(err.is(GenericDomain::get(), 0));
  std::cout << "testErrorBasic passed" << '\n';
}

void testErrorWithDomain() {
  CustomDomain custom;
  Error err(custom, 123, "Custom error message");
  assert(err.domain() == custom);
  assert(err.code() == 123);
  assert(err.message() == "Custom error message");
  assert(err.isDomain(custom));
  assert(err.is(custom, 123));
  assert(!err.isDomain(GenericDomain::get()));
  std::cout << "testErrorWithDomain passed" << '\n';
}

void testErrorToString() {
  Error err("Standard error");
  std::string errStr = err.toString();
  assert(errStr.find("[Generic:unknown error code: 0] Standard error") !=
         std::string::npos);

  CustomDomain custom;
  Error err2(custom, 1, "Msg");
  std::string errStr2 = err2.toString();
  assert(errStr2.find("[Custom:First error] Msg") != std::string::npos);
  std::cout << "testErrorToString passed" << '\n';
}

void testErrorStream() {
  Error err("Stream test");
  std::stringstream sstream;
  sstream << err;
  assert(sstream.str() == err.toString());
  std::cout << "testErrorStream passed" << '\n';
}

void testErrHelper() {
  auto res = err("Helper error");
  assert(res.isErr());
  assert(res.unwrapErr().message() == "Helper error");

  CustomDomain custom;
  auto res2 = err(custom, 404, "Not found");
  assert(res2.isErr());
  assert(res2.unwrapErr().is(custom, 404));
  assert(res2.unwrapErr().message() == "Not found");
  std::cout << "testErrHelper passed" << '\n';
}

void testFormat() {
#ifdef DCVB_HAS_STD_FORMAT
  Error err("Format test");
  std::string defaultFormat = std::format("{}", err);
  assert(defaultFormat == "[Generic:unknown error code: 0] Format test");

  std::string verboseFormat = std::format("{:v}", err);
  assert(verboseFormat.find("[Generic:unknown error code: 0] Format test") != std::string::npos);
  assert(verboseFormat.find("test-error.cpp:") != std::string::npos);
  std::cout << "testFormat passed" << '\n';
#else
  std::cout << "testFormat skipped (std::format not available)" << '\n';
#endif
}

void testWithContext() {
  Error base("file not found");

  // withContext prepends context to the message
  auto ctx = base.withContext("loading config");
  assert(ctx.message() == "loading config: file not found");

  // domain and code are preserved
  assert(ctx.domain() == GenericDomain::get());
  assert(ctx.code() == 0);

  // location is preserved from original
  assert(ctx.location().line() == base.location().line());

  // toString reflects the new message
  std::string str = ctx.toString();
  assert(str.find("loading config: file not found") != std::string::npos);

  // chaining: withContext on withContext prepends further
  auto nested = base.withContext("outer").withContext("inner");
  assert(nested.message() == "inner: outer: file not found");

  // works with custom domain
  CustomDomain custom;
  Error domErr(custom, 1, "disk error");
  auto domCtx = domErr.withContext("reading file");
  assert(domCtx.domain() == custom);
  assert(domCtx.code() == 1);
  assert(domCtx.message() == "reading file: disk error");

  std::cout << "testWithContext passed" << '\n';
}

auto main() -> int {
  try {
    testGenericDomain();
    testCustomDomain();
    testErrorBasic();
    testErrorWithDomain();
    testErrorToString();
    testErrorStream();
    testErrHelper();
    testFormat();
    testWithContext();

    std::cout << "All tests passed successfully!" << '\n';
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << '\n';
    return 1;
  }
  return 0;
}
