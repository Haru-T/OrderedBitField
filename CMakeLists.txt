cmake_minimum_required(VERSION 3.12)

project(OrderedBitField VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(ORDERED_BIT_FIELD_REF_BY_STR "enable member access by string literals" OFF)
option(ORDERED_BIT_FIELD_DISALLOW_OVERSIZED_FIELD "disallow fields larger than the base type" OFF)

add_library(OrderedBitField INTERFACE)
target_include_directories(OrderedBitField INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(OrderedBitField INTERFACE
  $<$<BOOL:${ORDERED_BIT_FIELD_REF_BY_STR}>:ORDERED_BIT_FIELD_REF_BY_STR=1>
  $<$<BOOL:${ORDERED_BIT_FIELD_DISALLOW_OVERSIZED_FIELD}>:ORDERED_BIT_FIELD_DISALLOW_OVERSIZED_FIELD=1>)
target_compile_features(OrderedBitField INTERFACE
  $<IF:$<BOOL:${ORDERED_BIT_FIELD_REF_BY_STR}>,cxx_std_20,cxx_std_17>)
