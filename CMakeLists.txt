cmake_minimum_required(VERSION 3.12)

project(OrderedBitField
  VERSION 0.1.0
  LANGUAGES CXX
  HOMEPAGE_URL "https://github.com/Haru-T/OrderedBitField"
  DESCRIPTION "Alternative to bit-fields with guaranteed field order and alignment")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_VERSION VERSION_LESS 3.21.0)
  if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(PROJECT_IS_TOP_LEVEL TRUE)
  else()
    set(PROJECT_IS_TOP_LEVEL FALSE)
  endif()
endif()

# options
include(CMakeDependentOption)
cmake_dependent_option(BUILD_TESTING "enable creation of tests." ON "PROJECT_IS_TOP_LEVEL" OFF)
option(ORDERED_BIT_FIELD_BUILD_TESTING "enable creation of OrderedBitField tests." ${BUILD_TESTING})
option(ORDERED_BIT_FIELD_BUILD_REF_BY_STR_TESTING "enable creation of tests related to member access by string literals." OFF)

# Main target
add_library(OrderedBitField INTERFACE)
target_include_directories(OrderedBitField INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>")
define_property(TARGET
  PROPERTY ORDERED_BIT_FIELD_REF_BY_STR
  BRIEF_DOCS "enable member access by string literals"
  FULL_DOCS "enable member access by string literals in OrderedBitField library")
define_property(TARGET
  PROPERTY ORDERED_BIT_FIELD_DISALLOW_OVERSIZED_FIELD
  BRIEF_DOCS "disallow fields larger than the base type"
  FULL_DOCS "disallow fields larger than the base type in OrderedBitField library")
target_compile_definitions(OrderedBitField INTERFACE
  $<$<BOOL:$<TARGET_PROPERTY:ORDERED_BIT_FIELD_REF_BY_STR>>:ORDERED_BIT_FIELD_REF_BY_STR=1>
  $<$<BOOL:$<TARGET_PROPERTY:ORDERED_BIT_FIELD_DISALLOW_OVERSIZED_FIELD>>:ORDERED_BIT_FIELD_DISALLOW_OVERSIZED_FIELD=1>)
target_compile_features(OrderedBitField INTERFACE
  $<IF:$<BOOL:$<TARGET_PROPERTY:ORDERED_BIT_FIELD_REF_BY_STR>>,cxx_std_20,cxx_std_17>)

# Package installation
if (PROJECT_IS_TOP_LEVEL)
  include(GNUInstallDirs)
  include(CMakePackageConfigHelpers)

  install(TARGETS OrderedBitField
    EXPORT OrderedBitFieldTargets)
  install(DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(EXPORT OrderedBitFieldTargets
    FILE OrderedBitFieldTargets.cmake
    NAMESPACE OrderedBitField::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OrderedBitField)
  configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/OrderedBitFieldConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/OrderedBitFieldConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OrderedBitField)
  write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/OrderedBitFieldConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)
  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/OrderedBitFieldConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/OrderedBitFieldConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OrderedBitField)
endif(PROJECT_IS_TOP_LEVEL)

# Testing
if(ORDERED_BIT_FIELD_BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.3.1)
  FetchContent_Populate(Catch2)
  add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
  include(CTest)
  include(Catch)

  add_executable(OrderedBitFieldTest EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/test/Alignment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/Operator.cpp)
  set_target_properties(OrderedBitFieldTest PROPERTIES
    ORDERED_BIT_FIELD_REF_BY_STR ${ORDERED_BIT_FIELD_BUILD_REF_BY_STR_TESTING})
  target_link_libraries(OrderedBitFieldTest
    PRIVATE OrderedBitField
    PRIVATE Catch2::Catch2WithMain)

  catch_discover_tests(OrderedBitFieldTest)
endif(ORDERED_BIT_FIELD_BUILD_TESTING)

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
