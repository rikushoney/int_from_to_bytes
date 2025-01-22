#ifndef INT_FROM_TO_BYTES_H
#define INT_FROM_TO_BYTES_H

#include <array>
#include <bit> // std::endian
#include <climits>
#include <concepts>
#include <cstddef> // std::byte
#include <type_traits>
#include <utility> // std::index_sequence

namespace iftb {

namespace detail_ {

constexpr std::size_t BITS_PER_BYTE = CHAR_BIT;

/// Helper struct for constructing bit masks.
template <std::size_t Select, std::unsigned_integral UInt> struct make_bitmask {
  static constexpr UInt value = [] {
    if constexpr (Select == 0) {
      // base case -- no bits selected
      return 0U;
    } else {
      // add a bit and shift bits up once
      return 1U | (make_bitmask<Select - 1, UInt>::value << 1U);
    }
  }();
};

template <std::size_t Width, std::unsigned_integral UInt>
constexpr UInt make_bitmask_v = make_bitmask<Width, UInt>::value;

/// Helper struct for constructing bit masks with offset.
template <std::size_t Width, std::size_t Offset, std::unsigned_integral UInt>
struct make_bitmask_offset {
  static constexpr UInt value = make_bitmask_v<Width, UInt> << Offset;
};

template <std::size_t Width, std::size_t Offset, std::unsigned_integral UInt>
constexpr UInt make_bitmask_offset_v =
    make_bitmask_offset<Width, Offset, UInt>::value;

/// Helper struct for calculating the bit offset of a byte index.
template <std::size_t Idx, std::endian Order, std::integral Int,
          std::size_t Width>
struct make_bit_offset {
  static_assert(Width <= sizeof(Int));
  static_assert(Idx >= 0);
  static_assert(Idx < Width);
  static constexpr std::size_t value = [] {
    if constexpr (Order == std::endian::little) {
      // [0][1][2][3]
      //  ^ -->
      return Idx;
    } else {
      // [0][1][2][3]
      //       <-- ^
      return Width - Idx - 1;
    }
  }() * BITS_PER_BYTE;
};

template <std::size_t Idx, std::endian Order, std::integral Int,
          std::size_t Width>
constexpr std::size_t make_bit_index_v =
    make_bit_offset<Idx, Order, Int, Width>::value;

/// Helper struct for constructing a byte shifted up.
template <std::size_t Idx, std::endian Order, std::integral Int,
          std::size_t Width>
constexpr Int shift_byte(std::byte b) {
  constexpr auto offset = make_bit_index_v<Idx, Order, Int, Width>;
  return std::to_integer<Int>(b) << offset;
}

/// Extract an integer from an arrangement of bytes.
template <std::endian Order, typename Bytes, std::integral Int,
          std::size_t Width, std::size_t... Idx>
constexpr Int extract_int(Bytes bytes, std::index_sequence<Idx...>) {
  static_assert(Width <= sizeof(Int));
  // return (bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | ...)
  return (shift_byte<Idx, Order, Int, Width>(bytes[Idx]) | ...);
}

/// Get the byte at an index of an integer.
template <std::size_t Idx, std::endian Order, std::integral Int,
          std::size_t Width>
constexpr std::byte get_byte(Int value) {
  static_assert(Idx < Width);
  using UInt = std::make_unsigned_t<Int>;
  constexpr auto offset = make_bit_index_v<Idx, Order, Int, Width>;
  constexpr auto mask = make_bitmask_offset_v<BITS_PER_BYTE, offset, UInt>;
  const auto bits = std::bit_cast<UInt>(value);
  return static_cast<std::byte>((bits & mask) >> offset);
}

/// Extract bytes from an integer.
template <std::endian Order, std::integral Int, std::size_t Width,
          std::size_t... Idx>
constexpr std::array<std::byte, Width>
extract_bytes(Int value, std::index_sequence<Idx...>) {
  static_assert(Width >= sizeof(Int));
  // return [value & 0xFF, (value & 0xFF00) >> 8, (value & 0xFF0000) >> 16, ...]
  return std::to_array({get_byte<Idx, Order, Int, Width>(value)...});
}

} // namespace detail_

/// Interpret an arrangement of bytes as an integer.
template <std::endian Order, std::integral Int, typename Bytes,
          std::size_t Width = sizeof(Int)>
constexpr Int from_bytes(Bytes bytes) {
  static_assert(Width <= sizeof(Int));
  return detail_::extract_int<Order, Bytes, Int, Width>(
      bytes, std::make_index_sequence<Width>{});
}

/// Deconstruct an integer into its byte representation.
template <std::endian Order, std::integral Int, std::size_t Width = sizeof(Int)>
constexpr std::array<std::byte, Width> to_bytes(Int value) {
  static_assert(Width >= sizeof(Int));
  return detail_::extract_bytes<Order, Int, Width>(
      value, std::make_index_sequence<Width>{});
}

} // namespace iftb

#endif
