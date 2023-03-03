# OrderedBitField

Ordered alternative to bit-fields in C++17/C++20

## Features

- Fixed-ordered fields independent of implementation
- Member access with custom enum or string literals
  - Access by string literals requires a C++20 feature (P1907R1: nontype template arguments)
- Support compound assignment operators

### Flag macros

- `ORDERED_BIT_FIELD_REF_BY_STR`: enable member access with string literals (requires C++20)
- `ORDERED_BIT_FIELD_DISALLOW_OVERSIZED_FIELD`: disallow fields larger than the base type (C style)

## Examples

### Member access with string literals

```cpp
#define ORDERED_BIT_FIELD_REF_BY_STR 1
#incldue <OrderedBitField/OrderedBitField.hpp>

using namespace OrderedBitField;

// ...

// Type definition
using F = BitField<uint8_t,                // storage type
                   Field<"a", 3>,          // mutable field "a" of size 3
                   ConstField<"b", 2, 3>,  // immutable field "b" of size 2 and value 3
                   Padding<2>,             // padding of size 2
                   Field<"c", 1, 1>>;      // mutable field "c" of size 1 and default value 1
F bit_field;

// Member access
auto a = get<"a">(bit_field);
a = 6;
auto b = get<"b">(bit_field);
// It fails because field "b" is const-qualified
// b = 1;
auto c = get<"c">(bit_field);
assert(static_cast<uint8_t>(c) == 2);

// Direct access to the storage
std::cout << std::bitset<8>(bit_field.Data[0]) << std::endl;
// Output: 10011110
```

### Member access with enum

```cpp
#define ORDERED_BIT_FIELD_REF_BY_STR 0
#incldue <OrderedBitField/OrderedBitField.hpp>

using namespace OrderedBitField;

// ...

// Type definition
enum class FieldName { A, B, C };

using F = BitField<uint8_t,                         // storage type
                   Field<FieldName::A, 3>,          // mutable field FieldName::A of size 3
                   ConstField<FieldName::B, 2, 3>,  // immutable field FieldName::B of size 2 and value 3
                   Padding<2>,                      // padding of size 2
                   Field<FieldName::C, 1, 1>>;      // mutable field FieldName::C of size 1 and default value 1
F bit_field;

// Member access
auto a = get<FieldName::A>(bit_field);
a = 6;
auto b = get<FieldName::B>(bit_field);
// It fails because field "b" is const-qualified
// b = 1;
auto c = get<FieldName::C>(bit_field);
assert(static_cast<uint8_t>(c) == 2);

// Direct access to the storage
std::cout << std::bitset<8>(bit_field.Data[0]) << std::endl;
// Output: 10011110
```

Note: The maximum value of the underlying type of the enum is used to represent unnamed fields (paddings).

## Specification

1. Fields are stored from least significant bit to most significant bit:
```cpp
BitField<uint8_t, Field<"a", 3>, Field<"b", 1>, Field<"c", 1>> BF;
//             7       5  4   3  2       0
//            +---------+---+---+---------+
// BF.Data[0] | (empty) |"c"|"b"|   "a"   |
//            +---------+---+---+---------+
```

2. If a field crosses a allocation unit boundary, it starts at the next allocation unit:
```cpp
BitField<uint8_t, Field<"a", 3>, Field<"b", 1>, Field<"c", 5>> BF_8x2;
//                 7         4  3  2       0
//                +-----------+---+---------+
// BF_8x2.Data[0] |  (empty)  |"b"|   "a"   |
//                +-----------+---+---------+
//                 7       5 4             0
//                +---------+---------------+
// BF_8x2.Data[1] | (empty) |      "c"      |
//                +---------+---------------+

BitField<uint8_t, Field<"a", 3>, Field<"b", 1>, Field<"c", 5>> BF_16x1;
//                  15        9 8             4  3  2       0
//                 +----...----+---------------+---+---------+
// BF_16x1.Data[0] |  (empty)  |      "c"      |"b"|   "a"   |
//                 +----...----+---------------+---+---------+
```

3. Fields of size zero break the padding and the next fields start from the next allocation unit:
```cpp
BitField<uint8_t, Field<"a", 3>, Field<"b", 1>, Padding<0>, Field<"c", 1>> BF;
//             7         4  3  2       0
//            +-----------+---+---------+
// BF.Data[0] |  (empty)  |"b"|   "a"   |
//            +-----------+---+---------+
//             7                   1  0
//            +---------------------+---+
// BF.Data[1] |       (empty)       |"c"|
//            +---------------------+---+
```

## Integration

Just copy `include/OrderedBitField` directory into your project and include `OrderedBitField/OrderedBitField.hpp`:

```cpp
// If you use string literals to access fields:
#define ORDERED_BIT_FIELD_REF_BY_STR 1

// If you disallow fields larger than the base type:
#define ORDERED_BIT_FIELD_DISALLOW_OVERSIZED_FIELD 1

#include <OrderedBitField/OrderedBitField.hpp>
```

## License

This library is dual-licensed under the Apache 2.0 and MIT terms.
