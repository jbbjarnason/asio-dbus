#include <array>
#include <tuple>

#include <fmt/format.h>

#include <boost/ut.hpp>
#include <forward_list>
#include <glaze/glaze.hpp>
#include <set>
#include <unordered_set>

#include <adbus/protocol/read.hpp>
#include <adbus/protocol/signature.hpp>

#include "common.hpp"

using namespace boost::ut;
using std::string_view_literals::operator""sv;
using std::string_literals::operator""s;

template <typename T>
  requires std::is_trivially_copyable_v<T>
struct generic_test {
  T expected{};
  std::array<std::uint8_t, sizeof(T)> buffer{};
};

int main() {
  using adbus::protocol::read_dbus_binary;

  "number types"_test =
      [](auto&& test) {
        using value_t = std::decay_t<decltype(test.expected)>;
        value_t value{};
        auto err = read_dbus_binary(value, test.buffer);
        expect(!err);
        expect(value == test.expected) << fmt::format("expected: {}, got: {}", test.expected, value);
      } |
      std::tuple{
        generic_test<std::uint8_t>{ .expected = 0x12, .buffer = { 0x12 } },
        generic_test<std::uint16_t>{ .expected = 0x1234, .buffer = { 0x34, 0x12 } },
        generic_test<std::uint32_t>{ .expected = 0x12345678, .buffer = { 0x78, 0x56, 0x34, 0x12 } },
        generic_test<std::uint64_t>{ .expected = 0x123456789abcdef0,
                                     .buffer = { 0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12 } },
        generic_test<std::int16_t>{ .expected = -0x1234, .buffer = { 0xcc, 0xed } },
        generic_test<std::int32_t>{ .expected = -0x12345678, .buffer = { 0x88, 0xa9, 0xcb, 0xed } },
        generic_test<std::int64_t>{ .expected = -0x123456789abcdef0,
                                    .buffer = { 0x10, 0x21, 0x43, 0x65, 0x87, 0xa9, 0xcb, 0xed } },
        generic_test<double>{ .expected = 1337.42, .buffer = { 0x48, 0xe1, 0x7a, 0x14, 0xae, 0xe5, 0x94, 0x40 } },
        generic_test<double>{ .expected = -1337.42, .buffer = { 0x48, 0xe1, 0x7a, 0x14, 0xae, 0xe5, 0x94, 0xc0 } },
      };

  "enum as number"_test =
      [](auto&& test) {
        using value_t = std::decay_t<decltype(test.expected)>;
        value_t value{};
        auto err = read_dbus_binary(value, test.buffer);
        expect(!err);
        // expect(value == test.expected) << fmt::format("expected: {}, got: {}", test.expected, value);
      } |
      std::tuple{
        generic_test<enum_as_number>{ .expected = enum_as_number::a, .buffer = { 0x01 } },
        generic_test<enum_as_number>{ .expected = enum_as_number::b, .buffer = { 0x02 } },
        generic_test<enum_as_number>{ .expected = enum_as_number::c, .buffer = { 0x03 } },
      };
}
