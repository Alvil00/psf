cmake_minimum_required(VERSION 4.3.2)
project(PolygonSymmetryFinder CXX)

# Option for my LSP: export compile_commands.json and link it into the source root
include(${PROJECT_SOURCE_DIR}/adm/cmake/CompileCommands.cmake)

# Standard definition
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(psf_core STATIC
  src/AxisAnchorPoint.cxx
  src/CommandLine.cxx
  src/CompatibilityClasses.cxx
  src/DihedralGroupElement.cxx
  src/MirrorAxesWriter.cxx
  src/MirrorAxis.cxx
  src/PointsReader.cxx
  src/Polygon.cxx
  src/SymbolString.cxx
  src/SymmetryFinder.cxx
)
target_include_directories(psf_core PUBLIC inc)

# MSVC's <cmath> declares M_PI and friends only with _USE_MATH_DEFINES; PUBLIC so psf and the tests get it too.
target_compile_definitions(psf_core PUBLIC $<$<CXX_COMPILER_ID:MSVC>:_USE_MATH_DEFINES>)

# Debug output of every stage to stdout (see inc/psf/DebugOutput.hxx); NDEBUG switches it off again.
option(PSF_DEBUG_OUTPUT "Print the intermediate results of every stage to stdout" OFF)
if(PSF_DEBUG_OUTPUT)
  target_compile_definitions(psf_core PUBLIC DEBUG_OUTPUT)
  if(CMAKE_BUILD_TYPE MATCHES "^(Release|RelWithDebInfo|MinSizeRel)$")
    message(WARNING "PSF_DEBUG_OUTPUT is ON, but the ${CMAKE_BUILD_TYPE} build defines NDEBUG, so nothing is printed")
  endif()
endif()

add_executable(psf src/main.cxx)
target_link_libraries(psf PRIVATE psf_core)


# Tests (PSF_BUILD_TESTS is set by conanfile.py)
option(PSF_BUILD_TESTS "Build unit tests" OFF)
if(PSF_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
