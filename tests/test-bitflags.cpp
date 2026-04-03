#include <cassert>
#include <dcvb/bitflags.hpp>
#include <dcvb/format.hpp>
#include <iostream>

using namespace dcvb;

enum class Perms : uint8_t { Read = 1, Write = 2, Exec = 4 };
enum class Options : uint32_t { None = 0, Verbose = 1, Debug = 2, Trace = 4 };

void testConstruction() {
  Bitflags<Perms> empty;
  assert(empty.none());
  assert(!empty.any());
  assert(!static_cast<bool>(empty));

  Bitflags<Perms> read(Perms::Read);
  assert(read.any());
  assert(read.has(Perms::Read));
  assert(!read.has(Perms::Write));

  auto fromRaw = Bitflags<Perms>::fromRaw(3);  // Read | Write
  assert(fromRaw.has(Perms::Read));
  assert(fromRaw.has(Perms::Write));
  assert(!fromRaw.has(Perms::Exec));

  std::cout << "testConstruction passed\n";
}

void testBitwiseOperators() {
  Bitflags<Perms> rw = Bitflags<Perms>(Perms::Read) | Perms::Write;
  assert(rw.has(Perms::Read));
  assert(rw.has(Perms::Write));
  assert(!rw.has(Perms::Exec));

  // Enum | Enum free operator
  auto rw2 = Perms::Read | Perms::Write;
  assert(rw == rw2);

  // operator &
  Bitflags<Perms> rwx = Bitflags<Perms>(Perms::Read) | Perms::Write | Perms::Exec;
  auto onlyWrite = rwx & Bitflags<Perms>(Perms::Write);
  assert(onlyWrite.has(Perms::Write));
  assert(!onlyWrite.has(Perms::Read));

  // operator ^
  auto toggled = rw ^ Bitflags<Perms>(Perms::Write);
  assert(toggled.has(Perms::Read));
  assert(!toggled.has(Perms::Write));

  // operator ~
  Bitflags<Perms> justRead(Perms::Read);
  auto notRead = ~justRead;
  assert(!notRead.has(Perms::Read));
  assert(notRead.has(Perms::Write));
  assert(notRead.has(Perms::Exec));

  std::cout << "testBitwiseOperators passed\n";
}

void testAssignmentOperators() {
  Bitflags<Perms> flags(Perms::Read);

  flags |= Perms::Write;
  assert(flags.has(Perms::Read));
  assert(flags.has(Perms::Write));

  flags &= Bitflags<Perms>(Perms::Write);
  assert(!flags.has(Perms::Read));
  assert(flags.has(Perms::Write));

  flags ^= Perms::Write;
  assert(flags.none());

  std::cout << "testAssignmentOperators passed\n";
}

void testHasAny() {
  auto rwx = Bitflags<Perms>(Perms::Read) | Perms::Write | Perms::Exec;
  auto rw  = Bitflags<Perms>(Perms::Read) | Perms::Write;

  // has: requires ALL bits
  assert(rwx.has(rw));         // rwx has both Read and Write
  assert(!rw.has(Perms::Exec));

  // hasAny: requires ANY bit
  auto wx = Bitflags<Perms>(Perms::Write) | Perms::Exec;
  assert(rw.hasAny(wx));    // rw has Write, which is in wx
  assert(!rw.hasAny(Perms::Exec));

  std::cout << "testHasAny passed\n";
}

void testMutationMethods() {
  Bitflags<Perms> flags;

  flags.set(Perms::Read).set(Perms::Write);
  assert(flags.has(Perms::Read));
  assert(flags.has(Perms::Write));

  flags.unset(Perms::Read);
  assert(!flags.has(Perms::Read));
  assert(flags.has(Perms::Write));

  flags.toggle(Perms::Read).toggle(Perms::Write);
  assert(flags.has(Perms::Read));
  assert(!flags.has(Perms::Write));

  std::cout << "testMutationMethods passed\n";
}

void testRawAndEquality() {
  Bitflags<Perms> rw = Perms::Read | Perms::Write;
  assert(rw.raw() == 3);

  Bitflags<Perms> rw2 = Perms::Read | Perms::Write;
  assert(rw == rw2);

  Bitflags<Perms> r(Perms::Read);
  assert(rw != r);

  std::cout << "testRawAndEquality passed\n";
}

void testFormat() {
#ifdef DCVB_HAS_STD_FORMAT
  Bitflags<Perms> rw = Perms::Read | Perms::Write;
  assert(std::format("{}", rw) == "Bitflags(3)");
  assert(std::format("{:x}", rw) == "Bitflags(0x3)");
  std::cout << "testFormat passed\n";
#else
  std::cout << "testFormat skipped (std::format not available)\n";
#endif
}

void testConstexpr() {
  constexpr auto rw = Bitflags<Perms>(Perms::Read) | Perms::Write;
  static_assert(rw.has(Perms::Read));
  static_assert(rw.has(Perms::Write));
  static_assert(!rw.has(Perms::Exec));
  static_assert(rw.raw() == 3);
  std::cout << "testConstexpr passed\n";
}

auto main() -> int {
  try {
    testConstruction();
    testBitwiseOperators();
    testAssignmentOperators();
    testHasAny();
    testMutationMethods();
    testRawAndEquality();
    testFormat();
    testConstexpr();

    std::cout << "All tests passed successfully!\n";
  } catch (const std::exception& exc) {
    std::cerr << "Test failed with exception: " << exc.what() << "\n";
    return 1;
  }
  return 0;
}
