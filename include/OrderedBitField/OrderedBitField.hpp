//===-- OrderedBitField.hpp - Order-guaranteed bit-fields -------*- C++ -*-===//
//
// Under the Apache License v2.0 and MIT License.
// SPDX-License-Identifier: Apache-2.0 OR MIT
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the definition of BitField class template, which is a
/// substitution for native bit-fields with order and argument-guaranteed fields
/// independent of compiler implementation.
///
/// \version 0.1.0
/// \author Haru-T <htgeek.with.insight+com.github@googlemail.com>
///
//===----------------------------------------------------------------------===//

#ifndef ORDERED_BIT_FIELD_HPP
#define ORDERED_BIT_FIELD_HPP

#if __cpp_nontype_template_args < 201911L
#if ORDERED_BIT_FIELD_REF_BY_STR
#warning                                                                       \
    "member access by string literals needs a C++20 feature (nontype template arguments)"
#undef ORDERED_BIT_FIELD_REF_BY_STR
#endif
#define ORDERED_BIT_FIELD_REF_BY_STR 0
#endif

#include <array>
#include <limits>
#include <tuple>
#include <type_traits>

#if ORDERED_BIT_FIELD_REF_BY_STR
#include <string_view>
#endif

namespace OrderedBitField {
/// Utilities.
namespace Util {
#if ORDERED_BIT_FIELD_REF_BY_STR
/// Utility class to handle string literal as non-type template parameters.
///
/// \note This class is not intended to be used by library users. This API may
/// have breaking change.
template <class CharT, std::size_t N> struct CharArray {
  using ValueType = CharT;
  constexpr CharArray(const CharT (&d)[N]) {
    for (std::size_t I = 0; I < N; ++I) {
      Data[I] = d[I];
    }
  }
  static constexpr std::size_t size() { return N; }
  constexpr std::basic_string_view<CharT> asStringView() const {
    return {Data};
  }

  constexpr bool operator==(std::basic_string_view<CharT> Rhs) const {
    return asStringView() == Rhs;
  }

  CharT Data[N];
};
#endif

/// Helper class to deduce underlying type.
///
/// \note This class is not intended to be used by library users. This API may
/// have breaking change.
template <class T> class UnderlyingTypeHelper {
  static constexpr auto impl() {
    if constexpr (std::is_enum_v<T>) {
      return std::underlying_type_t<T>{};
    } else {
      return T{};
    }
  }

public:
  using Type = decltype(impl());
};

/// Underlying type.
///
/// If T is an integral type, UnderlyingType<T> = T.
/// If T is an enum type, UnderlyingType<T> = std::underlying_type_t<T>.
///
/// \note This definition is not intended to be used by library users. This API
/// may have breaking change.
template <class T>
using UnderlyingType = typename UnderlyingTypeHelper<T>::Type;
} // namespace Util

#if ORDERED_BIT_FIELD_REF_BY_STR
/// %Field descriptors to access members by string literals.
///
/// If ORDERED_BIT_FIELD_REF_BY_STR is set, this namespace will be enabled as an
/// inline namespace.
inline namespace RefByStr {

/// Bit field descriptor.
///
/// \tparam T Name of the field.
/// \tparam W Width of the field.
/// \tparam D Default value of the field.
/// \tparam F Whether the value of the field is fixed or not.
template <Util::CharArray T, std::size_t W, auto D = 0, bool F = false>
struct Field {
  /// Name of the field.
  static constexpr auto Tag = T.asStringView();
  /// Width of the field.
  static constexpr std::size_t Width = W;
  /// Default value of the field.
  static constexpr auto DefaultValue = D;
  /// Whether the value of the field is fixed or not.
  static constexpr bool Fixed = F;
};

/// Bit field descriptor for const-qualified (fixed) field.
///
/// \tparam Tag Name of the field.
/// \tparam W Width of the field.
/// \tparam D Default value of the field.
template <Util::CharArray Tag, std::size_t W, auto D>
using ConstField = Field<Tag, W, D, true>;

/// Bit field descriptor for unused (unnamed) field.
///
/// \tparam W Width of the field.
/// \tparam CharT Character type of tags. You must set this if you use other
/// than `char` (e.g. `char8_t`, `wchar_t`, ...)
template <std::size_t W, class CharT = char>
using Padding = Field<Util::CharArray<CharT, 1>{{0}}, W, 0, true>;
} // namespace RefByStr
#endif

/// %Field descriptors to access members by enum.
///
/// If ORDERED_BIT_FIELD_REF_BY_STR is not set, this namespace will become an
/// inline namespace.
#if !ORDERED_BIT_FIELD_REF_BY_STR
inline
#endif
    namespace RefByEnum {
/// Bit field descriptor.
///
/// \tparam Tag Name of the field.
/// \tparam W Width of the field.
/// \tparam D Default value of the field.
/// \tparam F Whether the value of the field is fixed or not.
template <auto T, std::size_t W, auto D = 0, bool F = false,
          std::enable_if_t<std::is_enum_v<decltype(T)>, std::nullptr_t> =
              nullptr>
struct Field {
  /// Tag of the field.
  static constexpr auto Tag = T;
  /// Width of the field.
  static constexpr std::size_t Width = W;
  /// Default value of the field.
  static constexpr auto DefaultValue = D;
  /// Whether the value of the field is fixed or not.
  static constexpr bool Fixed = F;
};

/// Bit field descriptor for const-qualified (fixed) field.
///
/// \tparam Tag Name of the field.
/// \tparam W Width of the field.
/// \tparam D Default value of the field.
template <auto Tag, std::size_t W, auto D>
using ConstField = Field<Tag, W, D, true>;

/// Bit field descriptor for unused (unnamed) field.
///
/// \tparam EnumT Type of tags.
/// \tparam W Width of the field.
/// \note Enum value which corresponds to the maximum value of the underlying
/// type is used as the tag.
template <class EnumT, std::size_t W>
using Padding =
    Field<static_cast<EnumT>(
              std::numeric_limits<std::underlying_type_t<EnumT>>::max()),
          W, 0, true>;
} // namespace RefByEnum

/// Alignment-guaranteed bit fields.
///
/// \tparam BaseT Base (storage) type. Must be an integral or an enum class.
/// \tparam FirstField %Field descriptor.
/// \tparam Fields Successive field descriptors.
///
///
/// \code
/// // Type definition
/// using F = BitField<uint8_t,                // storage type
///                    Field<"a", 3>,          // mutable field "a" of size 3
///                    ConstField<"b", 2, 3>,  // immutable field "b" of size 2
///                                            // and value 3
///                    Padding<2>,             // padding of size 2
///                    Field<"c", 1, 1>>;      // mutable field "c" of size 1
///                                            // and default value 1
/// F bit_field;
///
/// // Member access
/// auto a = get<"a">(bit_field);
/// a = 6;
/// auto b = get<"b">(bit_field);
/// // It fails because field "b" is const-qualified
/// // b = 1;
/// auto c = get<"c">(bit_field);
/// assert(static_cast<uint8_t>(c) == 2);
///
/// // Direct access to the storage
/// std::cout << std::bitset<8>(bit_field.Field[0]) << std::endl;
/// // Output: 10011110
/// \endcode
template <class BaseT, class FirstField, class... Fields> class BitField {
  static_assert(std::is_integral_v<BaseT> || std::is_enum_v<BaseT>,
                "base type must be an integral type or an enum type");
  static_assert((std::conjunction_v<std::is_same<decltype(FirstField::Tag),
                                                 decltype(Fields::Tag)>...>),
                "types of tags of the all fields must be the same");
  static_assert(!std::is_integral_v<decltype(FirstField::Tag)>,
                "type of tags cannot be an integral type");

public:
  /// Base type of the fields.
  using FieldType = std::remove_reference_t<std::remove_const_t<BaseT>>;

  /// Type of tags.
  using TagT =
      std::remove_reference_t<std::remove_cv_t<decltype(FirstField::Tag)>>;

private:
  /// Underlying type of the fields. If FieldType is an enum type,
  /// UnderlyingType is std::underlying_type_t<FieldType>.
  using UnderlyingType = Util::UnderlyingType<FieldType>;

  /// Size of a field type in bits.
  static constexpr std::size_t FieldTypeBits =
      sizeof(FieldType) * std::numeric_limits<unsigned char>::digits;

#if ORDERED_BIT_FIELD_DISALLOW_OVERSIZED_FIELD
  static_assert(
      ((FirstField::Width <= FieldTypeBits) && ... &&
       (Fields::Width <= FieldTypeBits)),
      "no bit field larger than base type is allowed as in C language");
#endif

  /// Number of fields.
  static constexpr std::size_t NFields = sizeof...(Fields) + 1;

  /// List of widths for each field.
  static constexpr std::array<std::size_t, NFields> Width = {FirstField::Width,
                                                             Fields::Width...};

  /// List of field tags.
  static constexpr std::array<TagT, NFields> Tag = {FirstField::Tag,
                                                    Fields::Tag...};

  /// List of begining positions of each field.
  static constexpr std::array<std::size_t, NFields + 1> FieldBegin = []() {
    std::array<std::size_t, NFields + 1> B{};
    std::size_t BeginBit = 0;

    auto toSkipToNextUnit = [&BeginBit](std::size_t W) {
      return BeginBit / FieldTypeBits != (BeginBit + W - 1) / FieldTypeBits;
    };

    for (std::size_t I = 0; I < NFields; ++I) {
      if (toSkipToNextUnit(Width[I])) {
        BeginBit =
            ((BeginBit + FieldTypeBits - 1) / FieldTypeBits) * FieldTypeBits;
      }
      B[I] = BeginBit;
      if (Width[I] == 0) {
        BeginBit =
            (BeginBit + FieldTypeBits - 1) / FieldTypeBits * FieldTypeBits;
      } else {
        BeginBit += Width[I];
      }
    }
    B[NFields] = BeginBit;
    return B;
  }();

  /// List of bit masks.
  static constexpr std::array<FieldType, NFields> Mask = []() {
    std::array<FieldType, NFields> Mask{};
    for (std::size_t I = 0; I < NFields; ++I) {
      std::size_t S = FieldBegin[I] % FieldTypeBits;
      UnderlyingType M{};
      for (std::size_t J = 0; J < std::min(Width[I], FieldTypeBits); ++J) {
        M |= static_cast<UnderlyingType>(1 << S);
        ++S;
      }
      Mask[I] = static_cast<FieldType>(M);
    }
    return Mask;
  }();

  /// List of default values.
  static constexpr std::array<BaseT, NFields> DefaultValue = {
      static_cast<FieldType>(FirstField::DefaultValue),
      static_cast<FieldType>(Fields::DefaultValue)...};

  /// List of flags whether the fields are const-qualified.
  static constexpr std::array<bool, NFields> FieldFixed = {FirstField::Fixed,
                                                           Fields::Fixed...};

  /// Proxy object for each field in bit_field.
  ///
  /// \tparam FieldT Base type of the field.
  /// \tparam Shift Shift width.
  /// \tparam Mask Bit mask.
  /// \note This struct has a reference to BitField. Take care of dangling
  /// references.
  template <class FieldT, std::size_t Shift, BaseT Mask> class FieldProxy {
    //// Base type of the field.
    using FieldType = std::remove_reference_t<std::remove_const_t<FieldT>>;

    /// Underlying type of the field. If FieldType is an enum type,
    /// UnderlyingType is std::underlying_type_t<FieldType> if FieldType is an
    /// enum type.
    using UnderlyingType = Util::UnderlyingType<FieldType>;

    static_assert(static_cast<UnderlyingType>(Mask) > 0,
                  "cannot access member of size zero");

    // detection idioms
    template <class, class = std::void_t<>>
    struct PreIncrementable : std::false_type {};
    template <class T>
    struct PreIncrementable<T, std::void_t<decltype(++std::declval<T &>())>>
        : std::true_type {};

    template <class, class = std::void_t<>>
    struct PreDecrementable : std::false_type {};
    template <class T>
    struct PreDecrementable<T, std::void_t<decltype(--std::declval<T &>())>>
        : std::true_type {};

    template <class, class = std::void_t<>>
    struct PostIncrementable : std::false_type {};
    template <class T>
    struct PostIncrementable<T, std::void_t<decltype(std::declval<T &>()++)>>
        : std::true_type {};

    template <class, class = std::void_t<>>
    struct PostDecrementable : std::false_type {};
    template <class T>
    struct PostDecrementable<T, std::void_t<decltype(std::declval<T &>()--)>>
        : std::true_type {};

  public:
    constexpr operator FieldType() const {
      if constexpr (std::is_unsigned_v<UnderlyingType>) {
        return static_cast<FieldType>((static_cast<UnderlyingType>(Field) &
                                       static_cast<UnderlyingType>(Mask)) >>
                                      Shift);
      } else {
        constexpr std::size_t MaskMsbOffset = [] {
          for (std::size_t Offset = 0;
               Offset < sizeof(UnderlyingType) *
                            std::numeric_limits<unsigned char>::digits;
               ++Offset) {
            UnderlyingType M =
                1 << (sizeof(UnderlyingType) *
                          std::numeric_limits<unsigned char>::digits -
                      Offset - 1);
            if (static_cast<UnderlyingType>(Mask) & M)
              return Offset;
          }
          // never reaches here since Mask is nonzero
          return std::size_t{};
        }();
        return static_cast<FieldType>((static_cast<UnderlyingType>(Field) &
                                       static_cast<UnderlyingType>(Mask))
                                          << MaskMsbOffset >>
                                      (Shift + MaskMsbOffset));
      }
    }

    template <class T>
    constexpr auto operator=(T Rhs)
        -> decltype(std::declval<FieldType &>() = std::declval<T>(),
                    std::declval<FieldProxy<FieldT, Shift, Mask> &>()) {
      static_assert(!std::is_const_v<FieldT>,
                    "assignment of read-only memeber is not allowed");
      Field =
          static_cast<FieldType>((static_cast<UnderlyingType>(Field) &
                                  ~static_cast<UnderlyingType>(Mask)) |
                                 ((static_cast<UnderlyingType>(Rhs) << Shift) &
                                  static_cast<UnderlyingType>(Mask)));
      return *this;
    }

    template <class T>
    constexpr auto operator+=(T Rhs)
        -> decltype(std::declval<FieldType>() + std::declval<T>(),
                    std::declval<FieldProxy<FieldT, Shift, Mask> &>()) {
      static_assert(!std::is_const_v<FieldT>,
                    "assignment of read-only memeber is not allowed");
      Field = static_cast<FieldType>(
          (static_cast<UnderlyingType>(Field) &
           ~static_cast<UnderlyingType>(Mask)) |
          ((static_cast<FieldType>(*this) + Rhs) << Shift &
           static_cast<UnderlyingType>(Mask)));
      return *this;
    }

    template <class T>
    constexpr auto operator-=(T Rhs)
        -> decltype(std::declval<FieldType>() - std::declval<T>(),
                    std::declval<FieldProxy<FieldT, Shift, Mask> &>()) {
      static_assert(!std::is_const_v<FieldT>,
                    "assignment of read-only memeber is not allowed");
      Field = static_cast<FieldType>(
          (static_cast<UnderlyingType>(Field) &
           ~static_cast<UnderlyingType>(Mask)) |
          ((static_cast<FieldType>(*this) - Rhs) << Shift &
           static_cast<UnderlyingType>(Mask)));
      return *this;
    }

    template <class T>
    constexpr auto operator*=(T Rhs)
        -> decltype(std::declval<FieldType>() * std::declval<T>(),
                    std::declval<FieldProxy<FieldT, Shift, Mask> &>()) {
      static_assert(!std::is_const_v<FieldT>,
                    "assignment of read-only memeber is not allowed");
      Field = static_cast<FieldType>(
          (static_cast<UnderlyingType>(Field) &
           ~static_cast<UnderlyingType>(Mask)) |
          ((static_cast<FieldType>(*this) * Rhs) << Shift &
           static_cast<UnderlyingType>(Mask)));
      return *this;
    }

    template <class T>
    constexpr auto operator/=(T Rhs)
        -> decltype(std::declval<FieldType>() / std::declval<T>(),
                    std::declval<FieldProxy<FieldT, Shift, Mask> &>()) {
      static_assert(!std::is_const_v<FieldT>,
                    "assignment of read-only memeber is not allowed");
      Field = static_cast<FieldType>(
          (static_cast<UnderlyingType>(Field) &
           ~static_cast<UnderlyingType>(Mask)) |
          ((static_cast<FieldType>(*this) / Rhs) << Shift &
           static_cast<UnderlyingType>(Mask)));
      return *this;
    }

    template <class T>
    constexpr auto operator%=(T Rhs)
        -> decltype(std::declval<FieldType>() % std::declval<T>(),
                    std::declval<FieldProxy<FieldT, Shift, Mask> &>()) {
      static_assert(!std::is_const_v<FieldT>,
                    "assignment of read-only memeber is not allowed");
      Field = static_cast<FieldType>(
          (static_cast<UnderlyingType>(Field) &
           ~static_cast<UnderlyingType>(Mask)) |
          ((static_cast<FieldType>(*this) % Rhs) << Shift &
           static_cast<UnderlyingType>(Mask)));
      return *this;
    }

    template <class T>
    constexpr auto operator&=(T Rhs)
        -> decltype(std::declval<FieldType>() & std::declval<T>(),
                    std::declval<FieldProxy<FieldT, Shift, Mask> &>()) {
      static_assert(!std::is_const_v<FieldT>,
                    "assignment of read-only memeber is not allowed");
      Field =
          static_cast<FieldType>(static_cast<UnderlyingType>(Field) &
                                 (static_cast<UnderlyingType>(Rhs) << Shift |
                                  ~static_cast<UnderlyingType>(Mask)));
      return *this;
    }

    template <class T>
    constexpr auto operator|=(T Rhs)
        -> decltype(std::declval<FieldType>() | std::declval<T>(),
                    std::declval<FieldProxy<FieldT, Shift, Mask> &>()) {
      static_assert(!std::is_const_v<FieldT>,
                    "assignment of read-only memeber is not allowed");
      Field = static_cast<FieldType>(static_cast<UnderlyingType>(Field) |
                                     static_cast<UnderlyingType>(Rhs) << Shift &
                                         static_cast<UnderlyingType>(Mask));
      return *this;
    }

    template <class T>
    constexpr auto operator^=(T Rhs)
        -> decltype(std::declval<FieldType>() ^ std::declval<T>(),
                    std::declval<FieldProxy<FieldT, Shift, Mask> &>()) {
      static_assert(!std::is_const_v<FieldT>,
                    "assignment of read-only memeber is not allowed");
      Field =
          static_cast<FieldType>(static_cast<UnderlyingType>(Field) &
                                     ~static_cast<UnderlyingType>(Mask) |
                                 (static_cast<UnderlyingType>(Field) ^
                                  static_cast<UnderlyingType>(Rhs) << Shift) &
                                     static_cast<UnderlyingType>(Mask));
      return *this;
    }

    template <class T>
    constexpr auto operator<<=(T Rhs)
        -> decltype(std::declval<FieldType>() << std::declval<T>(),
                    std::declval<FieldProxy<FieldT, Shift, Mask> &>()) {
      static_assert(!std::is_const_v<FieldT>,
                    "assignment of read-only memeber is not allowed");
      Field = static_cast<FieldType>(static_cast<UnderlyingType>(Field) &
                                         ~static_cast<UnderlyingType>(Mask) |
                                     (static_cast<UnderlyingType>(Field) &
                                      static_cast<UnderlyingType>(Mask))
                                             << Rhs &
                                         static_cast<UnderlyingType>(Mask));
      return *this;
    }

    template <class T>
    constexpr auto operator>>=(T Rhs)
        -> decltype(std::declval<FieldType>() >> std::declval<T>(),
                    std::declval<FieldProxy<FieldT, Shift, Mask> &>()) {
      static_assert(!std::is_const_v<FieldT>,
                    "assignment of read-only memeber is not allowed");
      if constexpr (std::is_unsigned_v<UnderlyingType>) {
        Field = static_cast<FieldType>(static_cast<UnderlyingType>(Field) &
                                           ~static_cast<UnderlyingType>(Mask) |
                                       (static_cast<UnderlyingType>(Field) &
                                        static_cast<UnderlyingType>(Mask)) >>
                                               Rhs &
                                           static_cast<UnderlyingType>(Mask));
      } else {
        Field = static_cast<FieldType>(static_cast<UnderlyingType>(Field) &
                                           ~static_cast<UnderlyingType>(Mask) |
                                       static_cast<FieldType>(*this) << Shift >>
                                               Rhs &
                                           static_cast<UnderlyingType>(Mask));
      }
      return *this;
    }

    constexpr auto operator++() -> FieldProxy<FieldT, Shift, Mask> & {
      if constexpr (PreIncrementable<FieldType>::value) {
        static_assert(!std::is_const_v<FieldT>,
                      "assignment of read-only memeber is not allowed");
        return *this += 1;
      } else {
        static_assert([] { return false; }(),
                      "base type is not preincrementable");
        return *this;
      }
    }

    constexpr auto operator--() -> FieldProxy<FieldT, Shift, Mask> & {
      if constexpr (PreDecrementable<FieldType>::value) {
        static_assert(!std::is_const_v<BaseT>,
                      "assignment of read-only memeber is not allowed");
        return *this -= 1;
      } else {
        static_assert([] { return false; }(),
                      "base type is not predecrementable");
        return *this;
      }
    }

    constexpr FieldType operator++(int) {
      FieldType rv = static_cast<FieldType>(*this);
      if constexpr (PostIncrementable<FieldType>::value) {
        static_assert(!std::is_const_v<FieldT>,
                      "assignment of read-only memeber is not allowed");
        this->operator++();
      } else {
        static_assert([] { return false; }(),
                      "base type is not postincrementable");
      }
      return rv;
    }

    template <class> constexpr FieldType operator--(int);

    constexpr FieldType operator--(int) {
      FieldType rv = static_cast<FieldType>(*this);
      if constexpr (PostDecrementable<FieldType>::value) {
        static_assert(!std::is_const_v<FieldT>,
                      "assignment of read-only memeber is not allowed");
        this->operator--();
      } else {
        static_assert([] { return false; }(),
                      "base type is not postdecrementable");
      }
      return rv;
    }

  private:
    constexpr FieldProxy(FieldT &Field) : Field(Field) {}

    template <class, class, class...> friend struct BitField;

    FieldT &Field;
  };

public:
  /// Size of the storage.
  ///
  /// \returns Data.size()
  static constexpr std::size_t dataSize() {
    return (FieldBegin[NFields] + FieldTypeBits - 1) / FieldTypeBits;
  }

  /// %Data storage for bit fields.
  ///
  /// \note Array (not std::array) can be retrieved with Field.data().
  std::array<FieldType, dataSize()> Data = []()
#if __cpp_consteval
      consteval
#endif
  {
    std::array<FieldType, dataSize()> F{};

    for (std::size_t I = 0; I < NFields; ++I) {
      F[FieldBegin[I] / FieldTypeBits] = static_cast<FieldType>(
          (static_cast<UnderlyingType>(F[FieldBegin[I] / FieldTypeBits]) &
           ~static_cast<UnderlyingType>(Mask[I])) |
          ((static_cast<UnderlyingType>(DefaultValue[I])
            << (FieldBegin[I] % FieldTypeBits)) &
           static_cast<UnderlyingType>(Mask[I])));
    }
    return F;
  }
  ();

private:
#if ORDERED_BIT_FIELD_REF_BY_STR
  /// Find index of the field by tag.
  ///
  /// \tparam Query Name of the field which is been looking for.
  /// \returns Index of the field.
  template <Util::CharArray Query> static constexpr std::size_t index() {
    constexpr std::size_t Index = [] {
      for (std::size_t I = 0; I < NFields; ++I) {
        if (Query == Tag[I]) {
          return I;
        }
      }
      return NFields;
    }();
    static_assert(Index < NFields, "field not found");
    return Index;
  }
#endif

  /// Find index of the field by tag.
  ///
  /// \tparam Query Tag which is been looking for.
  /// \returns Index of the field.
  template <std::conditional_t<std::is_enum_v<TagT>, TagT, void *> Query>
  static constexpr std::size_t index() {
    constexpr std::size_t Index = [] {
      for (std::size_t I = 0; I < NFields; ++I) {
        if (Query == Tag[I]) {
          return I;
        }
      }
      return NFields;
    }();
    static_assert(Index < NFields, "field not found");
    return Index;
  }

  /// Get proxy object to the field by its index.
  ///
  /// \tparam I Index of the field.
  /// \returns Proxy object to the field.
  template <std::size_t I>
  constexpr auto get() -> std::conditional_t<
      FieldFixed[I],
      FieldProxy<const FieldType, FieldBegin[I] % FieldTypeBits, Mask[I]>,
      FieldProxy<FieldType, FieldBegin[I] % FieldTypeBits, Mask[I]>> {
    return {Data[FieldBegin[I] / FieldTypeBits]};
  }

  template <std::size_t I>
  constexpr std::enable_if_t<I >= NFields> get() = delete;

  /// Get proxy object to the field by its index.
  ///
  /// \tparam I Index of the field.
  /// \returns Proxy object to the field.
  template <std::size_t I>
  constexpr auto get() const
      -> FieldProxy<const FieldType, FieldBegin[I] % FieldTypeBits, Mask[I]> {
    return {Data[FieldBegin[I] / FieldTypeBits]};
  }

  template <std::size_t I>
  constexpr std::enable_if_t<I >= NFields> get() const = delete;

public:
#if ORDERED_BIT_FIELD_REF_BY_STR
  /// Get proxy object to the field by its tag.
  ///
  /// \tparam Query Name of the field.
  /// \returns Proxy object to the field.
  ///
  /// \note Use OrderedBitField::get for your convenience.
  /// \sa OrderedBitField::get
  template <Util::CharArray Query>
  constexpr auto get() -> decltype(get<index<Query>()>()) {
    return get<index<Query>()>();
  }

  /// Get proxy object to the field by its tag.
  ///
  /// \tparam Query Name of the field.
  /// \returns Proxy object to the field.
  ///
  /// \note Use OrderedBitField::get for your convenience.
  /// \sa OrderedBitField::get
  template <Util::CharArray Query>
  constexpr auto get() const -> decltype(get<index<Query>()>()) {
    return get<index<Query>()>();
  }
#endif

  /// Get proxy object to the field by its tag.
  ///
  /// \tparam Query Tag of the field.
  /// \returns Proxy object to the field.
  ///
  /// \note Use OrderedBitField::get for your convenience.
  /// \sa OrderedBitField::get
  template <std::conditional_t<std::is_enum_v<TagT>, TagT, void *> Query>
  constexpr auto get() -> decltype(get<index<Query>()>()) {
    return get<index<Query>()>();
  }

  /// Get proxy object to the field by its tag.
  ///
  /// \tparam Query Tag of the field.
  /// \returns Proxy object to the field.
  ///
  /// \note Use OrderedBitField::get for your convenience.
  /// \sa OrderedBitField::get
  template <std::conditional_t<std::is_enum_v<TagT>, TagT, void *> Query>
  constexpr auto get() const -> decltype(get<index<Query>()>()) {
    return get<index<Query>()>();
  }
};
#if ORDERED_BIT_FIELD_REF_BY_STR
/// Get proxy object to the field by its tag.
///
/// \tparam Query Name of the field.
/// \returns Proxy object to the field.
///
/// \code
///   using namespace OrderedBitfield;
///   BitField<std::byte, Field<"a", 4>, Field<"b", 2>, Field<"c", 2>> bf;
///   auto a = get<"a">(bf);
///   a = 3;
///   std::cout << std::bitset<8>{bf.Field} << std::endl; // 00000011
/// \endcode
///
/// \note The returned value has a reference to the BitField object. Watch for
/// dangling references.
template <Util::CharArray Query, class... Args,
          decltype(Query == std::declval<typename BitField<Args...>::TagT>(),
                   nullptr) = nullptr>
constexpr auto get(BitField<Args...> &BF) {
  return BF.template get<Query>();
}

/// Get proxy object to the field by its tag.
///
/// \tparam Query Name of the field.
/// \returns Proxy object to the field.
///
/// \code
///   using namespace OrderedBitfield;
///   BitField<std::byte, Field<"a", 4>, Field<"b", 2>, Field<"c", 2>> bf;
///   auto a = get<"a">(bf);
///   a = 3;
///   std::cout << std::bitset<8>{bf.Field} << std::endl; // 00000011
/// \endcode
///
/// \note The returned value has a reference to the BitField object. Watch for
/// dangling references.
template <Util::CharArray Query, class... Args,
          class = decltype(Query ==
                           std::declval<typename BitField<Args...>::TagT>())>
constexpr auto get(const BitField<Args...> &BF) {
  return BF.template get<Query>();
}

/// Field access to rvalue reference is not allowed.
template <Util::CharArray Query, class... Args>
constexpr auto get(BitField<Args...> &&) = delete;
#endif

/// Get proxy object to the field by its tag.
///
/// \tparam Query Tag of the field.
/// \returns Proxy object to the field.
///
/// \code
///   using namespace OrderedBitfield;
///   enum class Tag { A, B, C };
///   BitField<std::byte, Field<Tag::A, 4>, Field<Tag::B, 2>,
///            Field<Tag::C, 2>> bf;
///   auto a = get<Tag::A>(bf);
///   a = 3;
///   std::cout << std::bitset<8>{bf.Field} << std::endl; // 00000011
/// \endcode
///
/// \note The returned value has a reference to the BitField object. Watch for
/// dangling references.
template <auto Query, class... Args,
          std::enable_if_t<std::is_enum_v<typename BitField<Args...>::TagT>,
                           std::nullptr_t> = nullptr>
constexpr auto get(BitField<Args...> &BF) {
  return BF.template get<Query>();
}

/// Get proxy object to the field by its tag.
///
/// \tparam Query Tag of the field.
/// \returns Proxy object to the field.
///
/// \code
///   using namespace OrderedBitfield;
///   enum class Tag { A, B, C };
///   BitField<std::byte, Field<Tag::A, 4>, Field<Tag::B, 2>,
///            Field<Tag::C, 2>> bf;
///   auto a = get<Tag::A>(bf);
///   a = 3;
///   std::cout << std::bitset<8>{bf.Field} << std::endl; // 00000011
/// \endcode
///
/// \note The returned value has a reference to the BitField object. Watch for
/// dangling references.
template <auto Query, class... Args,
          std::enable_if_t<std::is_enum_v<typename BitField<Args...>::TagT>,
                           std::nullptr_t> = nullptr>
constexpr auto get(const BitField<Args...> &BF) {
  return BF.template get<Query>();
}

/// Field access to rvalue reference is not allowed.
template <auto Query, class... Args>
constexpr auto get(BitField<Args...> &&) = delete;
} // namespace OrderedBitField

#endif
