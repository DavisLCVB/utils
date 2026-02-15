#include <cassert>
#include <dcvb/error.hpp>
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

auto main() -> int {
  try {
    testGenericDomain();
    testCustomDomain();
    testErrorBasic();
    testErrorWithDomain();
    testErrorToString();
    testErrorStream();
    testErrHelper();

    std::cout << "All tests passed successfully!" << '\n';
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << '\n';
    return 1;
  }
  return 0;
}
