cmake_minimum_required(VERSION 3.12)

project(OrderedBitField VERSION 0.1.0 LANGUAGES CXX)

if(CMAKE_VERSION VERSION_LESS 3.21.0)
  if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(PROJECT_IS_TOP_LEVEL TRUE)
  else()
    set(PROJECT_IS_TOP_LEVEL FALSE)
  endif()
endif()

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

# Testing
include(CMakeDependentOption)
cmake_dependent_option(BUILD_TESTING "enable creation of tests." ON "PROJECT_IS_TOP_LEVEL" OFF)
option(ORDERED_BIT_FIELD_BUILD_TESTING "enable creation of OrderedBitField tests." ${BUILD_TESTING})

if(ORDERED_BIT_FIELD_BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.3.1)
  FetchContent_MakeAvailable(Catch2)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
  include(CTest)
  include(Catch)

  add_executable(OrderedBitFieldTest EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/test/Alignment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/Operator.cpp)
  target_link_libraries(OrderedBitFieldTest
    PRIVATE OrderedBitField
    PRIVATE Catch2::Catch2WithMain)
  target_compile_definitions(OrderedBitFieldTest PRIVATE
    $<$<BOOL:${ORDERED_BIT_FIELD_REF_BY_STR}>:ORDERED_BIT_FIELD_REF_BY_STR=1>)
endif(ORDERED_BIT_FIELD_BUILD_TESTING)

catch_discover_tests(OrderedBitFieldTest)

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
  set(DOXYGEN_PREDEFINED ORDERED_BIT_FIELD_REF_BY_STR=1
                         __cpp_nontype_template_args=201911L)
  doxygen_add_docs(OrderedBitFieldDoc
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    COMMENT "generate doxygen document")
endif(DOXYGEN_FOUND)
