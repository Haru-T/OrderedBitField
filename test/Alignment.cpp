//===-- test/Alignment.cpp - Test for BitField alignment --------*- C++ -*-===//
//
// Under the Apache License v2.0 and MIT License.
// SPDX-License-Identifier: Apache-2.0 OR MIT
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains unit test of alignment and field access for BitField.
///
/// \author Haru-T <htgeek.with.insight+com.github@googlemail.com>
///
//===----------------------------------------------------------------------===//

#include "OrderedBitField/OrderedBitField.hpp"

#include <cstddef>

#include <catch2/catch_template_test_macros.hpp>

using namespace OrderedBitField;
enum class Tag { A, B, C };

static_assert(std::numeric_limits<unsigned char>::digits == 8,
              "1 byte is not equal to 8 bits on this platform, breaking tests");

TEMPLATE_TEST_CASE("Alignment test (RefByEnum)", "[Alignment][RefByEnum]",
                   std::byte, std::uint8_t) {
  SECTION("sum of field width <= 1 byte") {
    BitField<TestType, RefByEnum::Field<Tag::A, 3>, RefByEnum::Field<Tag::B, 1>,
             RefByEnum::Field<Tag::C, 1>>
        BF;

    REQUIRE(BF.dataSize() == 1);
    get<Tag::A>(BF) = TestType{2};
    get<Tag::B>(BF) = TestType{0};
    get<Tag::C>(BF) = TestType{1};
    REQUIRE(BF.Data[0] == TestType{0b000'1'0'010});
  }

  SECTION("sum of field width > 1 byte") {
    BitField<TestType, RefByEnum::Field<Tag::A, 3>, RefByEnum::Field<Tag::B, 1>,
             RefByEnum::Field<Tag::C, 5>>
        BF;
    REQUIRE(BF.dataSize() == 2);
    get<Tag::A>(BF) = TestType{2};
    get<Tag::B>(BF) = TestType{0};
    get<Tag::C>(BF) = TestType{10};
    REQUIRE(BF.Data[0] == TestType{0b0000'0'010});
    REQUIRE(BF.Data[1] == TestType{0b000'01010});
  }

  SECTION("with non-zero size padding") {
    BitField<TestType, RefByEnum::Field<Tag::A, 3>, RefByEnum::Field<Tag::B, 1>,
             RefByEnum::Padding<Tag, 2>, RefByEnum::Field<Tag::C, 1>>
        BF;
    REQUIRE(BF.dataSize() == 1);
    get<Tag::A>(BF) = TestType{2};
    get<Tag::B>(BF) = TestType{0};
    get<Tag::C>(BF) = TestType{1};
    REQUIRE(BF.Data[0] == TestType{0b0'1'00'0'010});
  }

  SECTION("zero size field breaks up padding") {
    BitField<TestType, RefByEnum::Field<Tag::A, 3>, RefByEnum::Field<Tag::B, 1>,
             RefByEnum::Padding<Tag, 0>, RefByEnum::Field<Tag::C, 1>>
        BF;
    REQUIRE(BF.dataSize() == 2);
    get<Tag::A>(BF) = TestType{2};
    get<Tag::B>(BF) = TestType{0};
    get<Tag::C>(BF) = TestType{1};
    REQUIRE(BF.Data[0] == TestType{0b0000'0'010});
    REQUIRE(BF.Data[1] == TestType{0b0000000'1});
  }
}

TEST_CASE("Aligmnet test for multi-byte BaseT", "[Alignment][RefByEnum]") {
  BitField<std::uint16_t, RefByEnum::Field<Tag::A, 3>,
           RefByEnum::Field<Tag::B, 1>, RefByEnum::Field<Tag::C, 5>>
      BF;
  REQUIRE(BF.dataSize() == 1);
  get<Tag::A>(BF) = 2;
  get<Tag::B>(BF) = 0;
  get<Tag::C>(BF) = 10;
  REQUIRE(BF.Data[0] == 0b0000000'01010'0'010);
}

#if ORDERED_BIT_FIELD_REF_BY_STR
TEMPLATE_TEST_CASE("Alignment test (RefByStr)", "[Alignment][RefByStr]",
                   std::byte, std::uint8_t) {
  SECTION("sum of field width <= 1 byte") {
    BitField<TestType, RefByStr::Field<"A", 3>, RefByStr::Field<"B", 1>,
             RefByStr::Field<"C", 1>>
        BF;

    REQUIRE(BF.dataSize() == 1);
    get<"A">(BF) = TestType{2};
    get<"B">(BF) = TestType{0};
    get<"C">(BF) = TestType{1};
    REQUIRE(BF.Data[0] == TestType{0b000'1'0'010});
  }

  SECTION("sum of field width > 1 byte") {
    BitField<TestType, RefByStr::Field<"A", 3>, RefByStr::Field<"B", 1>,
             RefByStr::Field<"C", 5>>
        BF;
    REQUIRE(BF.dataSize() == 2);
    get<"A">(BF) = TestType{2};
    get<"B">(BF) = TestType{0};
    get<"C">(BF) = TestType{10};
    REQUIRE(BF.Data[0] == TestType{0b0000'0'010});
    REQUIRE(BF.Data[1] == TestType{0b000'01010});
  }

  SECTION("with non-zero size padding") {
    BitField<TestType, RefByStr::Field<"A", 3>, RefByStr::Field<"B", 1>,
             RefByStr::Padding<2>, RefByStr::Field<"C", 1>>
        BF;
    REQUIRE(BF.dataSize() == 1);
    get<"A">(BF) = TestType{2};
    get<"B">(BF) = TestType{0};
    get<"C">(BF) = TestType{1};
    REQUIRE(BF.Data[0] == TestType{0b0'1'00'0'010});
  }

  SECTION("zero size field breaks up padding") {
    BitField<TestType, RefByStr::Field<"A", 3>, RefByStr::Field<"B", 1>,
             RefByStr::Padding<0>, RefByStr::Field<"C", 1>>
        BF;
    REQUIRE(BF.dataSize() == 2);
    get<"A">(BF) = TestType{2};
    get<"B">(BF) = TestType{0};
    get<"C">(BF) = TestType{1};
    REQUIRE(BF.Data[0] == TestType{0b0000'0'010});
    REQUIRE(BF.Data[1] == TestType{0b0000000'1});
  }
}

TEST_CASE("Aligmnet test for multi-byte BaseT", "[Alignment][RefByStr]") {
  BitField<std::uint16_t, RefByStr::Field<"A", 3>, RefByStr::Field<"B", 1>,
           RefByStr::Field<"C", 5>>
      BF;
  REQUIRE(BF.dataSize() == 1);
  get<"A">(BF) = 2;
  get<"B">(BF) = 0;
  get<"C">(BF) = 10;
  REQUIRE(BF.Data[0] == 0b0000000'01010'0'010);
}
#endif
