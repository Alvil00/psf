find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(psf_tests
  AxisAnchorPointTest.cxx
  CommandLineTest.cxx
  CompatibilityClassesTest.cxx
  CyclicPalindromesTest.cxx
  DihedralGroupElementTest.cxx
  Matrix2dTest.cxx
  MirrorAxesWriterTest.cxx
  MirrorAxisTest.cxx
  Point2dTest.cxx
  PointsReaderTest.cxx
  PolygonTest.cxx
  SymbolStringTest.cxx
  SymmetryFinderTest.cxx
  Vector2dTest.cxx
)
target_link_libraries(psf_tests PRIVATE psf_core GTest::gtest_main)

gtest_discover_tests(psf_tests)
