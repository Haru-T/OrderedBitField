//===-- test/Operator.cpp - Test for bit/arithmetic operators ---*- C++ -*-===//
//
// Under the Apache License v2.0 and MIT License.
// SPDX-License-Identifier: Apache-2.0 OR MIT
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains unit test of bit/arithmetic operators for fields.
///
/// \author Haru-T <htgeek.with.insight+com.github@googlemail.com>
///
//===----------------------------------------------------------------------===//

#include "OrderedBitField/OrderedBitField.hpp"

#include <cstddef>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>

using namespace OrderedBitField;
enum class Tag { A, B };

// sign extension helper
template <class T> constexpr T SignExt(T Val) {
  return Val << (sizeof(T) * std::numeric_limits<unsigned char>::digits - 4) >>
         (sizeof(T) * std::numeric_limits<unsigned char>::digits - 4);
}

TEMPLATE_TEST_CASE("Arithmetic operators test", "[Operator]", std::byte,
                   std::uint8_t, std::uint16_t, std::int32_t) {
  BitField<TestType, RefByEnum::Field<Tag::A, 4>, RefByEnum::Field<Tag::B, 4>>
      BF;

  REQUIRE(BF.dataSize() == 1);

  auto A = GENERATE(range(0, 15));
  auto B = GENERATE(range(0, 15));
  if constexpr (std::is_signed_v<TestType>) {
    A -= 8;
    B -= 8;
  }
  const auto A0 = static_cast<TestType>(A);
  const auto B0 = static_cast<TestType>(B);
  get<Tag::A>(BF) = A0;
  get<Tag::B>(BF) = B0;

  SECTION("Type cast") {
    REQUIRE(static_cast<TestType>(get<Tag::A>(BF)) == A0);
    REQUIRE(get<Tag::B>(BF) == B0);
  }

  SECTION("operator=") {
    auto I = GENERATE(range(0, 15));
    if constexpr (std::is_signed_v<TestType>) {
      I -= 8;
    }
    auto Rhs = static_cast<TestType>(I);

    get<Tag::A>(BF) = Rhs;
    REQUIRE(get<Tag::A>(BF) == Rhs);
    REQUIRE(get<Tag::B>(BF) == B0);

    get<Tag::B>(BF) = Rhs;
    REQUIRE(get<Tag::A>(BF) == Rhs);
    REQUIRE(get<Tag::B>(BF) == Rhs);
  }

  SECTION("operator|=") {
    auto I = GENERATE(range(0, 15));
    if constexpr (std::is_signed_v<TestType>) {
      I -= 8;
    }
    auto Rhs = static_cast<TestType>(I);

    get<Tag::A>(BF) |= Rhs;
    REQUIRE(get<Tag::A>(BF) == (A0 | Rhs));
    REQUIRE(get<Tag::B>(BF) == B0);

    get<Tag::B>(BF) |= Rhs;
    REQUIRE(get<Tag::A>(BF) == (A0 | Rhs));
    REQUIRE(get<Tag::B>(BF) == (B0 | Rhs));
  }

  SECTION("operator&=") {
    auto I = GENERATE(range(0, 15));
    if constexpr (std::is_signed_v<TestType>) {
      I -= 8;
    }
    auto Rhs = static_cast<TestType>(I);

    get<Tag::A>(BF) &= Rhs;
    REQUIRE(get<Tag::A>(BF) == (A0 & Rhs));
    REQUIRE(get<Tag::B>(BF) == B0);

    get<Tag::B>(BF) &= Rhs;
    REQUIRE(get<Tag::A>(BF) == (A0 & Rhs));
    REQUIRE(get<Tag::B>(BF) == (B0 & Rhs));
  }

  SECTION("operator^=") {
    auto I = GENERATE(range(0, 15));
    if constexpr (std::is_signed_v<TestType>) {
      I -= 8;
    }
    auto Rhs = static_cast<TestType>(I);

    get<Tag::A>(BF) ^= Rhs;
    REQUIRE(get<Tag::A>(BF) == (A0 ^ Rhs));
    REQUIRE(get<Tag::B>(BF) == B0);

    get<Tag::B>(BF) ^= Rhs;
    REQUIRE(get<Tag::A>(BF) == (A0 ^ Rhs));
    REQUIRE(get<Tag::B>(BF) == (B0 ^ Rhs));
  }

  // before C++20, behaviors of shift operators for signed integer operands
  // depends on implementation
#if __cplusplus < 202002
  if constexpr (!std::is_signed_v<TestType>) {
#endif
    SECTION("operator<<=") {
      auto Rhs = GENERATE(range(0, 3));

      get<Tag::A>(BF) <<= Rhs;
      REQUIRE(get<Tag::A>(BF) == SignExt<TestType>(A0 << Rhs & TestType{0x0f}));
      REQUIRE(get<Tag::B>(BF) == B0);

      get<Tag::B>(BF) <<= Rhs;
      REQUIRE(get<Tag::A>(BF) == SignExt<TestType>(A0 << Rhs & TestType{0x0f}));
      REQUIRE(get<Tag::B>(BF) == SignExt<TestType>(B0 << Rhs & TestType{0x0f}));
    }

    SECTION("operator>>=") {
      auto Rhs = GENERATE(range(0, 3));

      get<Tag::A>(BF) >>= Rhs;
      REQUIRE(get<Tag::A>(BF) == (A0 >> Rhs));
      REQUIRE(get<Tag::B>(BF) == B0);

      get<Tag::B>(BF) >>= Rhs;
      REQUIRE(get<Tag::A>(BF) == (A0 >> Rhs));
      REQUIRE(get<Tag::B>(BF) == (B0 >> Rhs));
    }
#if __cplusplus < 202002
  }
#endif

  if constexpr (std::is_integral_v<TestType>) {
    SECTION("operator+=") {
      auto I = GENERATE(range(0, 15));
      if constexpr (std::is_signed_v<TestType>) {
        I -= 8;
      }
      auto Rhs = static_cast<TestType>(I);

      get<Tag::A>(BF) += Rhs;
      REQUIRE(get<Tag::A>(BF) == SignExt<TestType>((A0 + Rhs) & 0x0f));
      REQUIRE(get<Tag::B>(BF) == B0);

      get<Tag::B>(BF) += Rhs;
      REQUIRE(get<Tag::A>(BF) == SignExt<TestType>((A0 + Rhs) & 0x0f));
      REQUIRE(get<Tag::B>(BF) == SignExt<TestType>((B0 + Rhs) & 0x0f));
    }

    SECTION("operator-=") {
      auto I = GENERATE(range(0, 15));
      if constexpr (std::is_signed_v<TestType>) {
        I -= 8;
      }
      auto Rhs = static_cast<TestType>(I);

      get<Tag::A>(BF) -= Rhs;
      REQUIRE(get<Tag::A>(BF) == SignExt<TestType>((A0 - Rhs) & 0x0f));
      REQUIRE(get<Tag::B>(BF) == B0);

      get<Tag::B>(BF) -= Rhs;
      REQUIRE(get<Tag::A>(BF) == SignExt<TestType>((A0 - Rhs) & 0x0f));
      REQUIRE(get<Tag::B>(BF) == SignExt<TestType>((B0 - Rhs) & 0x0f));
    }

    SECTION("operator*=") {
      auto I = GENERATE(range(0, 15));
      auto Rhs = static_cast<TestType>(I);

      get<Tag::A>(BF) *= Rhs;
      REQUIRE(get<Tag::A>(BF) == SignExt<TestType>((A0 * Rhs) & 0x0f));
      REQUIRE(get<Tag::B>(BF) == B0);

      get<Tag::B>(BF) *= Rhs;
      REQUIRE(get<Tag::A>(BF) == SignExt<TestType>((A0 * Rhs) & 0x0f));
      REQUIRE(get<Tag::B>(BF) == SignExt<TestType>((B0 * Rhs) & 0x0f));
    }

    SECTION("operator/=") {
      auto I = GENERATE(range(1, 16));
      auto Rhs = static_cast<TestType>(I);

      get<Tag::A>(BF) /= Rhs;
      REQUIRE(get<Tag::A>(BF) == A0 / Rhs);
      REQUIRE(get<Tag::B>(BF) == B0);

      get<Tag::B>(BF) /= Rhs;
      REQUIRE(get<Tag::A>(BF) == A0 / Rhs);
      REQUIRE(get<Tag::B>(BF) == B0 / Rhs);
    }

    SECTION("operator%=") {
      auto I = GENERATE(range(1, 16));
      auto Rhs = static_cast<TestType>(I);

      get<Tag::A>(BF) %= Rhs;
      REQUIRE(get<Tag::A>(BF) == A0 % Rhs);
      REQUIRE(get<Tag::B>(BF) == B0);

      get<Tag::B>(BF) %= Rhs;
      REQUIRE(get<Tag::A>(BF) == A0 % Rhs);
      REQUIRE(get<Tag::B>(BF) == B0 % Rhs);
    }
  }
}
