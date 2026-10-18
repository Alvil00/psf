find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(psf_tests
  Point2dTest.cxx
  PointsReaderTest.cxx
  Vector2dTest.cxx
)
target_link_libraries(psf_tests PRIVATE psf_core GTest::gtest_main)

gtest_discover_tests(psf_tests)
