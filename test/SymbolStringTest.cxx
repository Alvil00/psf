#include "psf/SymbolString.hxx"

#include "psf/CyclicPalindromes.hxx"
#include "psf/PolygonError.hxx"

#include "TauSymmetricPolygons.hxx"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <random>
#include <vector>


namespace {

using psf::CyclicPalindromes;
using psf::Point2d;
using psf::PolygonError;
using psf::Symbol;
using psf::SymbolString;
using Centers = std::vector<std::size_t>;
using Points  = std::vector<Point2d>;

Centers CandidateCenters(const psf::Polygon& thePolygon, double theTolerance) {
  return CyclicPalindromes::Centers(SymbolString::Build(thePolygon, theTolerance));
}

Centers CandidateCenters(const Points& thePoints, double theTolerance) {
  return CandidateCenters(psf::Polygon(thePoints), theTolerance);
}

Points RegularPolygon(std::size_t theCount, double theRadius) {
  Points aPolygon;
  for (std::size_t i = 0; i < theCount; ++i) {
    const double anAngle = 2 * M_PI * static_cast<double>(i) / static_cast<double>(theCount);
    aPolygon.emplace_back(theRadius * std::cos(anAngle), theRadius * std::sin(anAngle));
  }
  return aPolygon;
}

//! Builds the symbol string of thePolygon and expects it to report overflow at input vertex theVertex.
void ExpectTooLarge(const psf::Polygon& thePolygon, std::size_t theVertex) {
  try {
    SymbolString::Build(thePolygon, 0.0);
    FAIL() << "expected PolygonError";
  } catch (const PolygonError& anError) {
    EXPECT_EQ(anError.Kind(), PolygonError::ErrorKind::TooLarge);
    EXPECT_EQ(anError.VertexIndex(), theVertex);
  }
}

}


TEST(SymbolStringTest, RejectsOverflowingCross) {
  // A square of side L with corners clipped by s. Cleanup drops the four vertices that end a long edge, because
  // the squared chord between their neighbours overflows; the remaining square's crosses are about L² = 1e320.
  // Input point 0 is one of the dropped vertices, so cleaned vertex 0 is input point 1.
  const double L = 1e160;
  const double s = 1e146;
  ExpectTooLarge(psf::Polygon({Point2d(L, 0), Point2d(L + s, s), Point2d(L + s, L), Point2d(L, L + s),
                               Point2d(s, L + s), Point2d(0, L), Point2d(0, s), Point2d(s, 0)},
                              0.0),
                 1);
}

TEST(SymbolStringTest, RejectsOverflowingLength) {
  // A right triangle with legs 1e154: every dot and cross is at most 1e308, but the hypotenuse's squared length
  // is 2e308. Input point 1 duplicates point 0, so cleaned vertex 1, where the hypotenuse starts, is input point 2.
  const double a = 1e154;
  ExpectTooLarge(psf::Polygon({Point2d(0, a), Point2d(0, a), Point2d(0, 0), Point2d(a, a)}, 0.0), 2);
}

TEST(SymbolStringTest, InterleavesVertexAndEdgeSymbols) {
  // Right triangle 3-4-5: edge lengths 3, 5, 4 all differ, so edge symbols must differ.
  const psf::Polygon aTriangle({Point2d(0, 0), Point2d(3, 0), Point2d(0, 4)});
  const std::vector<Symbol> aSymbols = SymbolString::Build(aTriangle, 1e-9);
  ASSERT_EQ(aSymbols.size(), 6u);
  EXPECT_NE(aSymbols[1], aSymbols[3]);
  EXPECT_NE(aSymbols[3], aSymbols[5]);
  EXPECT_NE(aSymbols[1], aSymbols[5]);
  EXPECT_EQ(aSymbols[1].mySecond, 0u);
}

TEST(SymbolStringTest, ScaleneTriangleHasNoCandidate) {
  EXPECT_TRUE(CandidateCenters({Point2d(0, 0), Point2d(4, 0), Point2d(1, 3)}, 1e-6).empty());
}

TEST(SymbolStringTest, IsoscelesTriangleHasOneCandidateThroughApex) {
  // Apex is vertex 2, element 4 of the symbol string; centres are reported modulo N = 3, so it is centre 1.
  EXPECT_EQ(CandidateCenters({Point2d(-1, 0), Point2d(1, 0), Point2d(0, 3)}, 1e-6), (Centers{1}));
}

TEST(SymbolStringTest, RectangleHasTwoCandidatesThroughEdgeMidpoints) {
  const Points aRectangle{Point2d(0, 0), Point2d(2, 0), Point2d(2, 1), Point2d(0, 1)};
  EXPECT_EQ(CandidateCenters(aRectangle, 1e-6), (Centers{1, 3}));
}

TEST(SymbolStringTest, SquareHasFourCandidates) {
  const Points aSquare{Point2d(0, 0), Point2d(1, 0), Point2d(1, 1), Point2d(0, 1)};
  EXPECT_EQ(CandidateCenters(aSquare, 1e-9), (Centers{0, 1, 2, 3}));
}

TEST(SymbolStringTest, ComputedRegularPolygonsHaveEveryCandidate) {
  // Coordinates come from cos/sin, so the polygon is symmetric only up to their rounding (~1e-13 here);
  // a tolerance a little above that must find every axis.
  for (const std::size_t aCount : {3u, 5u, 6u, 7u, 12u, 31u}) {
    const Centers aCenters = CandidateCenters(RegularPolygon(aCount, 1000.0), 1e-9);
    EXPECT_EQ(aCenters.size(), aCount) << aCount << "-gon";
  }
}

TEST(SymbolStringTest, NeverMissesTheAxisOfATauSymmetricPolygon) {
  // The axis through vertex 0 is element 0 of the symbol string, so centre 0 must be reported.
  std::mt19937 aGenerator(20261018);
  int aCheckedCount = 0;
  for (int anIter = 0; anIter < 4000; ++anIter) {
    const psf_test::TauSymmetricCase aCase = psf_test::MakeTauSymmetricCase(aGenerator, anIter);
    const std::optional<psf::Polygon> aPolygon = psf_test::TryExactPolygon(aCase.myPoints, aCase.myTolerance);
    if (!aPolygon) {
      continue;
    }
    const Centers aCenters = CandidateCenters(*aPolygon, aCase.myTolerance);
    ASSERT_FALSE(aCenters.empty()) << "iteration " << anIter;
    ASSERT_EQ(aCenters.front(), 0u) << "iteration " << anIter;
    ++aCheckedCount;
  }
  EXPECT_GT(aCheckedCount, 3600) << "too many random polygons were skipped";
}
