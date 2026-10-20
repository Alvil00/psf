#include "psf/SymmetryFinder.hxx"

#include "psf/CyclicPalindromes.hxx"
#include "psf/SymbolString.hxx"

#include "TauSymmetricPolygons.hxx"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <random>
#include <vector>


namespace {

using psf::AxisStatus;
using psf::MirrorAxis;
using psf::Point2d;
using psf::Polygon;
using psf::SymmetryFinder;
using Axes   = std::vector<MirrorAxis>;
using Points = std::vector<Point2d>;

Points RegularPolygon(std::size_t theCount, double theRadius) {
  Points aPoints;
  for (std::size_t i = 0; i < theCount; ++i) {
    const double anAngle = 2 * M_PI * static_cast<double>(i) / static_cast<double>(theCount);
    aPoints.emplace_back(theRadius * std::cos(anAngle), theRadius * std::sin(anAngle));
  }
  return aPoints;
}

bool IsAllVerified(const Axes& theAxes) {
  return std::all_of(theAxes.begin(), theAxes.end(),
                     [](const MirrorAxis& theAxis) { return theAxis.Status() == AxisStatus::Verified; });
}

}


TEST(SymmetryFinderTest, ScaleneTriangleHasNoAxis) {
  EXPECT_TRUE(SymmetryFinder::Find(Polygon({Point2d(0, 0), Point2d(4, 0), Point2d(1, 3)})).empty());
}

TEST(SymmetryFinderTest, IsoscelesTriangleAxisRunsFromBaseMidpointToApex) {
  const Axes anAxes = SymmetryFinder::Find(Polygon({Point2d(-1, 0), Point2d(1, 0), Point2d(0, 3)}));
  ASSERT_EQ(anAxes.size(), 1u);
  const MirrorAxis& anAxis = anAxes.front();
  EXPECT_EQ(anAxis.Status(), AxisStatus::Verified);
  EXPECT_FALSE(anAxis.First().IsVertex());
  EXPECT_EQ(anAxis.First().FirstVertex(), 0u);
  EXPECT_EQ(anAxis.First().SecondVertex(), 1u);
  EXPECT_EQ(anAxis.First().Point(), Point2d(0, 0));
  EXPECT_TRUE(anAxis.Second().IsVertex());
  EXPECT_EQ(anAxis.Second().FirstVertex(), 2u);
}

TEST(SymmetryFinderTest, SquareHasFourVerifiedAxes) {
  const Axes anAxes = SymmetryFinder::Find(Polygon({Point2d(0, 0), Point2d(1, 0), Point2d(1, 1), Point2d(0, 1)}));
  ASSERT_EQ(anAxes.size(), 4u);
  EXPECT_TRUE(IsAllVerified(anAxes));
  // First axis: the diagonal from vertex 0 to vertex 2.
  EXPECT_TRUE(anAxes[0].First().IsVertex());
  EXPECT_EQ(anAxes[0].First().FirstVertex(), 0u);
  EXPECT_EQ(anAxes[0].Second().FirstVertex(), 2u);
}

TEST(SymmetryFinderTest, RectangleAxesRunBetweenEdgeMidpoints) {
  const Axes anAxes = SymmetryFinder::Find(Polygon({Point2d(0, 0), Point2d(2, 0), Point2d(2, 1), Point2d(0, 1)}));
  ASSERT_EQ(anAxes.size(), 2u);
  EXPECT_TRUE(IsAllVerified(anAxes));
  EXPECT_EQ(anAxes[0].First().Point(), Point2d(1, 0));
  EXPECT_EQ(anAxes[0].Second().Point(), Point2d(1, 1));
  EXPECT_EQ(anAxes[1].First().Point(), Point2d(2, 0.5));
  EXPECT_EQ(anAxes[1].Second().Point(), Point2d(0, 0.5));
}

TEST(SymmetryFinderTest, ReportsOriginalIndicesThroughCleanup) {
  // Input vertex 1 is collinear and input vertex 5 repeats vertex 0; the polygon keeps inputs 0, 2, 3, 4.
  const Polygon aPolygon({Point2d(0, 0), Point2d(1, 0), Point2d(2, 0), Point2d(2, 2), Point2d(0, 2), Point2d(0, 0)});
  const Axes anAxes = SymmetryFinder::Find(aPolygon);
  ASSERT_EQ(anAxes.size(), 4u);
  EXPECT_EQ(anAxes[1].First().FirstVertex(), 0u);   // midpoint of the bottom edge: inputs 0 and 2
  EXPECT_EQ(anAxes[1].First().SecondVertex(), 2u);
  EXPECT_EQ(anAxes[1].First().Point(), Point2d(1, 0));
}

TEST(SymmetryFinderTest, RegularPolygonsAreVerifiedByTheGroupShortcut) {
  for (const std::size_t aCount : {5u, 6u, 12u, 31u, 64u}) {
    const Axes anAxes = SymmetryFinder::Find(Polygon(RegularPolygon(aCount, 1000.0)), 1e-6);
    EXPECT_EQ(anAxes.size(), aCount) << aCount << "-gon";
    EXPECT_TRUE(IsAllVerified(anAxes)) << aCount << "-gon";
    // The group shortcut reports one shared bound; the per-axis fallback would measure each axis separately.
    for (const auto& anAxis : anAxes) {
      EXPECT_EQ(anAxis.Deviation(), anAxes.front().Deviation());
    }
  }
}

TEST(SymmetryFinderTest, FallbackRejectsFalseCandidatesOfAnEllipse) {
  // A finely sampled ellipse: neighbouring features differ by less than τ, so every class merges and every centre
  // is a candidate. Only the two true axes (through vertices 0 and 20, and through vertices 10 and 30) survive.
  Points aPoints;
  for (int i = 0; i < 40; ++i) {
    const double anAngle = 2 * M_PI * i / 40;
    aPoints.emplace_back(1.2 * std::cos(anAngle), std::sin(anAngle));
  }
  const Polygon aPolygon(aPoints);
  const double aTau = 5e-3;
  ASSERT_GT(psf::CyclicPalindromes::Centers(psf::SymbolString::Build(aPolygon, aTau)).size(), 2u);

  const Axes anAxes = SymmetryFinder::Find(aPolygon, aTau);
  ASSERT_EQ(anAxes.size(), 2u);
  EXPECT_TRUE(IsAllVerified(anAxes));
  EXPECT_EQ(anAxes[0].First().FirstVertex(), 0u);
  EXPECT_EQ(anAxes[0].Second().FirstVertex(), 20u);
  EXPECT_EQ(anAxes[1].First().FirstVertex(), 10u);
  EXPECT_EQ(anAxes[1].Second().FirstVertex(), 30u);
}

TEST(SymmetryFinderTest, SlightlySkewedTriangleIsAmbiguous) {
  // The apex is 0.01 off the base's perpendicular bisector. Reflecting across the line from the base midpoint to the
  // apex misses by about 0.0067 > τ, so the axis is not proven; but that is within τ plus the estimation margin.
  const Axes anAxes = SymmetryFinder::Find(Polygon({Point2d(-1, 0), Point2d(1, 0), Point2d(0.01, 3)}), 0.005);
  ASSERT_EQ(anAxes.size(), 1u);
  EXPECT_EQ(anAxes.front().Status(), AxisStatus::Ambiguous);
  EXPECT_GT(anAxes.front().Deviation(), 0.005);
}

TEST(SymmetryFinderTest, NeverRejectsTheAxisOfATauSymmetricPolygon) {
  std::mt19937 aGenerator(20261018);
  int aCheckedCount = 0;
  for (int anIter = 0; anIter < 4000; ++anIter) {
    const psf_test::TauSymmetricCase aCase = psf_test::MakeTauSymmetricCase(aGenerator, anIter);
    const std::optional<Polygon> aPolygon = psf_test::TryExactPolygon(aCase.myPoints, aCase.myTolerance);
    if (!aPolygon) {
      continue;
    }
    const Axes anAxes = SymmetryFinder::Find(*aPolygon, aCase.myTolerance);
    const bool hasAxis = std::any_of(anAxes.begin(), anAxes.end(), [](const MirrorAxis& theAxis) {
      return theAxis.First().IsVertex() && theAxis.First().FirstVertex() == 0;
    });
    ASSERT_TRUE(hasAxis) << "iteration " << anIter;
    ++aCheckedCount;
  }
  EXPECT_GT(aCheckedCount, 3600) << "too many random polygons were skipped";
}

TEST(SymmetryFinderTest, NeverRejectsAnyAxisOfAPerturbedRegularPolygon) {
  // Every vertex moves by up to τ/2, so every axis of the regular polygon stays τ-symmetric. The group shortcut may
  // fail at τ/2; the per-axis fallback must still keep all of them.
  std::mt19937 aGenerator(7);
  std::uniform_real_distribution<double> aUnit(0.0, 1.0);
  for (int anIter = 0; anIter < 500; ++anIter) {
    const auto aCount = std::uniform_int_distribution<std::size_t>(3, 24)(aGenerator);
    // Perturbations deeper than half a corner's bend could dent the polygon, which cleanup rightly rejects.
    const double aBendDepth  = 1.0 - std::cos(M_PI / static_cast<double>(aCount));
    const double aTau        = std::min(std::pow(10.0, -1.0 - 6.0 * aUnit(aGenerator)), 0.5 * aBendDepth);
    const bool isOnBoundary  = aUnit(aGenerator) < 0.5;
    const double aRotation   = 2 * M_PI * aUnit(aGenerator);
    const psf::Vector2d aTranslation{100 * aUnit(aGenerator), 100 * aUnit(aGenerator)};

    Points aPoints = RegularPolygon(aCount, 1.0);
    for (auto& aPoint : aPoints) {
      aPoint += psf_test::RandomShift(aGenerator, 0.999 * aTau / 2, isOnBoundary);
      aPoint = psf_test::Move(aPoint, aRotation, aTranslation);
    }
    const std::optional<Polygon> aPolygon = psf_test::TryExactPolygon(aPoints, aTau);
    ASSERT_TRUE(aPolygon) << "iteration " << anIter;
    EXPECT_EQ(SymmetryFinder::Find(*aPolygon, aTau).size(), aCount) << "iteration " << anIter;
  }
}
