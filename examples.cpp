#include "int_from_to_bytes.h"

#include <algorithm>
#include <array>
#include <assert.h>
#include <cstddef>
#include <print>

using namespace iftb;

#define STRINGYFY(...) #__VA_ARGS__

#define TEST_CASE_STRING(order, type, value, ...)                              \
  "testing (" STRINGYFY(order) ", " STRINGYFY(type) "): " STRINGYFY(           \
      value) " == {" STRINGYFY(__VA_ARGS__) "}"

#define TEST_CASE_PRINT(order, type, value, ...)                               \
  std::println("{}", TEST_CASE_STRING(order, type, value, __VA_ARGS__))

#define TEST_CASE_FROM(order, type, expected, ...)                             \
  assert((from_bytes<std::endian::order, type>(                                \
              std::to_array({__VA_ARGS__})) == expected))

#define TEST_BYTES_EQ(b1, b2) assert((std::ranges::equal(b1, b2)))

#define TEST_CASE_TO(order, type, value, ...)                                  \
  TEST_BYTES_EQ((to_bytes<std::endian::order, type>(value)),                   \
                std::to_array({__VA_ARGS__}))

#define TEST_CASE(order, type, value, ...)                                     \
  TEST_CASE_PRINT(order, type, value, __VA_ARGS__);                            \
  TEST_CASE_FROM(order, type, value, __VA_ARGS__);                             \
  TEST_CASE_TO(order, type, value, __VA_ARGS__)

void test_examples() {
  constexpr auto ff = std::byte{0xFF};
  TEST_CASE(little, int, -1, ff, ff, ff, ff);

  constexpr auto zero = std::byte{0};
  TEST_CASE(little, unsigned, 16711935, ff, zero, ff, zero);
  TEST_CASE(big, int, -16711936, ff, zero, ff, zero);
  TEST_CASE(little, int, -16711936, zero, ff, zero, ff);
  TEST_CASE(big, unsigned, 16711935, zero, ff, zero, ff);
  TEST_CASE(little, unsigned, 65535, ff, ff, zero, zero);
  TEST_CASE(big, int, -65536, ff, ff, zero, zero);
  TEST_CASE(little, int, -65536, zero, zero, ff, ff);
  TEST_CASE(big, unsigned, 65535, zero, zero, ff, ff);

  constexpr auto eb = std::byte{0xEB};
  constexpr auto _32 = std::byte{0x32};
  constexpr auto a4 = std::byte{0xA4};
  constexpr auto f8 = std::byte{0xF8};
  TEST_CASE(little, int, -123456789, eb, _32, a4, f8);
  TEST_CASE(big, int, -123456789, f8, a4, _32, eb);
  constexpr auto eb_32_a4_f8 = to_bytes<std::endian::little, int>(-123456789);
  constexpr auto min_123456789 =
      from_bytes<std::endian::little, int>(eb_32_a4_f8);
  assert(min_123456789 == -123456789);
}

int main() {
  test_examples();
  return 0;
}
