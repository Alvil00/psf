find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(psf_tests
  Point2dTest.cxx
  Vector2dTest.cxx
)
target_include_directories(psf_tests PRIVATE ${PROJECT_SOURCE_DIR}/inc)
target_link_libraries(psf_tests PRIVATE GTest::gtest_main)

gtest_discover_tests(psf_tests)
