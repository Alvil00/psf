#include "psf/Polygon.hxx"

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>


namespace {

using psf::Point2d;
using psf::Polygon;
using psf::PolygonError;
using psf::Vector2d;
using ErrorKind = PolygonError::ErrorKind;
using Indices   = std::vector<std::size_t>;
using Points    = std::vector<Point2d>;

//! Builds a Polygon from thePoints and returns the kind of PolygonError it throws; fails the test if none is thrown.
ErrorKind ErrorOf(const Points& thePoints, double theCollinearity = psf::BaseTraits::Precision) {
  try {
    Polygon aPolygon(thePoints, theCollinearity);
  } catch (const PolygonError& anError) {
    return anError.Kind();
  }
  ADD_FAILURE() << "no PolygonError thrown";
  return ErrorKind::TooFewVertices;
}

}


TEST(PolygonTest, KeepsACleanSquareAsIs) {
  const Points aSquare{Point2d(0, 0), Point2d(1, 0), Point2d(1, 1), Point2d(0, 1)};
  const Polygon aPolygon(aSquare);
  EXPECT_EQ(aPolygon.Vertices(), aSquare);
  EXPECT_EQ(aPolygon.OriginalIndices(), (Indices{0, 1, 2, 3}));
}

TEST(PolygonTest, CleansPointsPassedByRvalue) {
  // A duplicate and a collinear vertex, so both cleanup steps run on the moved-in points.
  Points aPoints{Point2d(0, 0), Point2d(0, 0), Point2d(1, 0), Point2d(2, 0), Point2d(2, 2), Point2d(0, 2)};
  const Polygon aPolygon(std::move(aPoints));
  EXPECT_EQ(aPolygon.Vertices(), (Points{Point2d(0, 0), Point2d(2, 0), Point2d(2, 2), Point2d(0, 2)}));
  EXPECT_EQ(aPolygon.OriginalIndices(), (Indices{0, 3, 4, 5}));
}

TEST(PolygonTest, AcceptsClockwiseOrder) {
  const Polygon aPolygon({Point2d(0, 0), Point2d(0, 1), Point2d(1, 1), Point2d(1, 0)});
  EXPECT_EQ(aPolygon.Size(), 4u);
}

TEST(PolygonTest, CounterClockwiseSquareIsCounterClockwise) {
  const Polygon aPolygon({Point2d(0, 0), Point2d(1, 0), Point2d(1, 1), Point2d(0, 1)});
  EXPECT_TRUE(aPolygon.IsCounterClockwise());
}

TEST(PolygonTest, ClockwiseSquareIsNotCounterClockwise) {
  const Polygon aPolygon({Point2d(0, 0), Point2d(0, 1), Point2d(1, 1), Point2d(1, 0)});
  EXPECT_FALSE(aPolygon.IsCounterClockwise());
}

TEST(PolygonTest, VertexWrapsAroundTheBoundary) {
  const Polygon aPolygon({Point2d(0, 0), Point2d(1, 0), Point2d(1, 1), Point2d(0, 1)});
  EXPECT_EQ(aPolygon.Vertex(3), Point2d(0, 1));
  EXPECT_EQ(aPolygon.Vertex(aPolygon.Size()), aPolygon.Vertex(0));
  EXPECT_EQ(aPolygon.Vertex(2 * aPolygon.Size() + 1), aPolygon.Vertex(1));
}

TEST(PolygonTest, EdgeVectorRunsToTheNextVertexAndClosesTheBoundary) {
  const Polygon aPolygon({Point2d(0, 0), Point2d(2, 0), Point2d(2, 1), Point2d(0, 1)});
  EXPECT_EQ(aPolygon.EdgeVector(0), Vector2d(2, 0));
  EXPECT_EQ(aPolygon.EdgeVector(3), Vector2d(0, -1));
  EXPECT_EQ(aPolygon.EdgeVector(aPolygon.Size() + 3), aPolygon.EdgeVector(3));
}

TEST(PolygonTest, EdgesFollowTheCleanedVertices) {
  // The duplicate and the collinear vertex are gone before the edges are built.
  const Polygon aPolygon({Point2d(0, 0), Point2d(0, 0), Point2d(1, 0), Point2d(2, 0), Point2d(2, 2), Point2d(0, 2)});
  const std::vector<Vector2d> anExpected{Vector2d(2, 0), Vector2d(0, 2), Vector2d(-2, 0), Vector2d(0, -2)};
  EXPECT_EQ(aPolygon.Edges(), anExpected);
}

TEST(PolygonTest, VertexCentroidAveragesTheCleanedVertices) {
  // The mean of the four vertices, not the area centroid.
  // The duplicate is merged first, so it does not count twice.
  const Polygon aPolygon({Point2d(0, 0), Point2d(0, 0), Point2d(3, 0), Point2d(2, 2), Point2d(0, 2)});
  const Point2d aCentroid = aPolygon.VertexCentroid();
  EXPECT_DOUBLE_EQ(aCentroid.X(), 1.25);
  EXPECT_DOUBLE_EQ(aCentroid.Y(), 1.0);
}

TEST(PolygonTest, RelocatesWithoutThrowing) {
  // std::vector<Polygon> moves its elements when it grows only if moving cannot throw; otherwise it copies them.
  static_assert(std::is_nothrow_move_constructible_v<Polygon>);
  static_assert(std::is_nothrow_move_assignable_v<Polygon>);
}

TEST(PolygonTest, DropsClosingDuplicate) {
  const Polygon aPolygon({Point2d(0, 0), Point2d(1, 0), Point2d(0, 1), Point2d(0, 0)});
  EXPECT_EQ(aPolygon.OriginalIndices(), (Indices{0, 1, 2}));
}

TEST(PolygonTest, DropsNearClosingDuplicate) {
  const Polygon aPolygon({Point2d(0, 0), Point2d(1, 0), Point2d(0, 1), Point2d(1e-10, 0)});
  EXPECT_EQ(aPolygon.OriginalIndices(), (Indices{0, 1, 2}));
}

TEST(PolygonTest, MergesRepeatedPointsWithoutLosingTheCorner) {
  // Judged against their neighbours, both copies of (1, 0) would lie on a line through the other copy;
  // merging them first keeps the corner.
  const Polygon aPolygon({Point2d(0, 0), Point2d(1, 0), Point2d(1, 0), Point2d(1, 0), Point2d(0, 1)});
  EXPECT_EQ(aPolygon.Vertices(), (Points{Point2d(0, 0), Point2d(1, 0), Point2d(0, 1)}));
  EXPECT_EQ(aPolygon.OriginalIndices(), (Indices{0, 1, 4}));
}

TEST(PolygonTest, DuplicateChainIsComparedWithTheKeptPoint) {
  // With r = 1: 0.6 is within r of the kept 0 and is dropped; 1.2 is not, although it is within r of 0.6.
  const Polygon aPolygon({Point2d(0, 0), Point2d(0.6, 0), Point2d(1.2, 0), Point2d(10, 0), Point2d(0, 10)}, 1.0);
  EXPECT_EQ(aPolygon.OriginalIndices(), (Indices{0, 3, 4}));
}

TEST(PolygonTest, RemovesCollinearVertices) {
  // Two vertices on the bottom edge and one on the left edge.
  const Polygon aPolygon(
    {Point2d(0, 0), Point2d(1, 0), Point2d(2, 0), Point2d(3, 0), Point2d(3, 3), Point2d(0, 3), Point2d(0, 1.5)});
  EXPECT_EQ(aPolygon.OriginalIndices(), (Indices{0, 3, 4, 5}));
}

TEST(PolygonTest, CollinearThresholdIsADistanceToTheNeighbourLine) {
  const Points aPoints{Point2d(0, 0), Point2d(1, -0.05), Point2d(2, 0), Point2d(2, 2), Point2d(0, 2)};
  EXPECT_EQ(Polygon(aPoints, 0.06).Size(), 4u);
  EXPECT_EQ(Polygon(aPoints, 0.04).Size(), 5u);
}

TEST(PolygonTest, SpikeAlongTheNeighbourLineIsNotCollinear) {
  // Vertex 1 lies on the line through its neighbours, but 99 beyond the segment between them; dropping it would
  // silently turn this input into a unit square.
  const Points aSpike{Point2d(0, 0), Point2d(100, 0), Point2d(1, 0), Point2d(1, 1), Point2d(0, 1)};
  try {
    Polygon aPolygon(aSpike);
    FAIL() << "no PolygonError thrown";
  } catch (const PolygonError& anError) {
    EXPECT_EQ(anError.Kind(), ErrorKind::NotConvex);
    EXPECT_EQ(anError.VertexIndex(), 1u);
  }
}

TEST(PolygonTest, SpikeVerdictDoesNotDependOnNoiseInNearlyCoincidentNeighbours) {
  // The neighbours of the spike vertex 1 are 1e-10 apart; the direction of that gap must not decide its fate.
  EXPECT_EQ(ErrorOf({Point2d(0, 0), Point2d(100, 0), Point2d(1e-10, 0), Point2d(0, 1)}), ErrorKind::NotConvex);
  EXPECT_EQ(ErrorOf({Point2d(0, 0), Point2d(100, 0), Point2d(0, 1e-10), Point2d(-1, 1)}), ErrorKind::NotConvex);
}

TEST(PolygonTest, TooFewVerticesAfterCleanup) {
  EXPECT_EQ(ErrorOf({}), ErrorKind::TooFewVertices);
  EXPECT_EQ(ErrorOf({Point2d(0, 0), Point2d(1, 0)}), ErrorKind::TooFewVertices);
  EXPECT_EQ(ErrorOf({Point2d(0, 0), Point2d(1, 0), Point2d(0, 0)}), ErrorKind::TooFewVertices);
  EXPECT_EQ(ErrorOf({Point2d(0, 0), Point2d(1, 0), Point2d(2, 0)}), ErrorKind::TooFewVertices);
}

TEST(PolygonTest, DentIsNotConvex) {
  // Vertex 2 points inwards.
  const Points anArrow{Point2d(0, 0), Point2d(4, 0), Point2d(2, 1), Point2d(4, 4), Point2d(0, 4)};
  try {
    Polygon aPolygon(anArrow);
    FAIL() << "no PolygonError thrown";
  } catch (const PolygonError& anError) {
    EXPECT_EQ(anError.Kind(), ErrorKind::NotConvex);
    EXPECT_EQ(anError.VertexIndex(), 2u);
  }
}

TEST(PolygonTest, SlightDentBeyondThresholdIsNotConvex) {
  const Points aPoints{Point2d(0, 0), Point2d(1, 0.05), Point2d(2, 0), Point2d(2, 2), Point2d(0, 2)};
  EXPECT_EQ(ErrorOf(aPoints, 0.04), ErrorKind::NotConvex);
  EXPECT_EQ(Polygon(aPoints, 0.06).Size(), 4u);
}

TEST(PolygonTest, BowTieIsNotConvex) {
  EXPECT_EQ(ErrorOf({Point2d(0, 0), Point2d(1, 1), Point2d(1, 0), Point2d(0, 1)}), ErrorKind::NotConvex);
}

TEST(PolygonTest, PentagramIsNotSimple) {
  Points aStar;
  for (int i = 0; i < 5; ++i) {
    const double anAngle = M_PI / 2 + 4 * M_PI * i / 5;
    aStar.emplace_back(std::cos(anAngle), std::sin(anAngle));
  }
  EXPECT_EQ(ErrorOf(aStar), ErrorKind::NotSimple);
}
