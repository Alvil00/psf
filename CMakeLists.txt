cmake_minimum_required(VERSION 4.3.2)
project(PolygonSymmetryFinder CXX)

# Option for my LSP: export compile_commands.json and link it into the source root
include(${PROJECT_SOURCE_DIR}/adm/cmake/CompileCommands.cmake)

# Standard definition
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(psf src/main.cxx)
target_include_directories(psf PRIVATE inc)


# Tests (PSF_BUILD_TESTS is set by conanfile.py)
option(PSF_BUILD_TESTS "Build unit tests" OFF)
if(PSF_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
