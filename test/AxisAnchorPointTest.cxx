#include "psf/AxisAnchorPoint.hxx"

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>


namespace {

using psf::AxisAnchorPoint;
using psf::Point2d;
using psf::Polygon;

//! The square (0, 0), (2, 0), (2, 2), (0, 2): N = 4, so its symbol elements run from 0 to 7.
Polygon Square() {
  return Polygon(std::vector<Point2d>{Point2d(0, 0), Point2d(2, 0), Point2d(2, 2), Point2d(0, 2)});
}

}


TEST(AxisAnchorPointTest, DefaultIsVertexZeroAtTheOrigin) {
  const AxisAnchorPoint aPoint;
  EXPECT_EQ(aPoint.Point(), Point2d(0, 0));
  EXPECT_EQ(aPoint.FirstVertex(), 0u);
  EXPECT_EQ(aPoint.SecondVertex(), 0u);
  EXPECT_TRUE(aPoint.IsVertex());
}

TEST(AxisAnchorPointTest, PartsAreKeptAsGiven) {
  const AxisAnchorPoint aPoint(Point2d(0.5, -1), 3, 7);
  EXPECT_EQ(aPoint.Point(), Point2d(0.5, -1));
  EXPECT_EQ(aPoint.FirstVertex(), 3u);
  EXPECT_EQ(aPoint.SecondVertex(), 7u);
  EXPECT_TRUE(aPoint.IsEdgeCenter());
}

TEST(AxisAnchorPointTest, EvenElementIsAVertex) {
  const AxisAnchorPoint aPoint(Square(), 4);
  EXPECT_EQ(aPoint.Point(), Point2d(2, 2));
  EXPECT_EQ(aPoint.FirstVertex(), 2u);
  EXPECT_EQ(aPoint.SecondVertex(), 2u);
  EXPECT_TRUE(aPoint.IsVertex());
}

TEST(AxisAnchorPointTest, OddElementIsAnEdgeMidpoint) {
  const AxisAnchorPoint aPoint(Square(), 1);
  EXPECT_EQ(aPoint.Point(), Point2d(1, 0));
  EXPECT_EQ(aPoint.FirstVertex(), 0u);
  EXPECT_EQ(aPoint.SecondVertex(), 1u);
  EXPECT_TRUE(aPoint.IsEdgeCenter());
}

TEST(AxisAnchorPointTest, LastOddElementIsTheClosingEdge) {
  const AxisAnchorPoint aPoint(Square(), 7);
  EXPECT_EQ(aPoint.Point(), Point2d(0, 1));
  EXPECT_EQ(aPoint.FirstVertex(), 3u);
  EXPECT_EQ(aPoint.SecondVertex(), 0u);
}

TEST(AxisAnchorPointTest, ElementWrapsModuloTwiceTheSize) {
  const Polygon aSquare = Square();
  for (std::size_t anElement = 0; anElement < 8; ++anElement) {
    const AxisAnchorPoint aPoint(aSquare, anElement);
    const AxisAnchorPoint aWrapped(aSquare, anElement + 8);
    EXPECT_EQ(aWrapped.Point(), aPoint.Point()) << "element " << anElement;
    EXPECT_EQ(aWrapped.FirstVertex(), aPoint.FirstVertex()) << "element " << anElement;
    EXPECT_EQ(aWrapped.SecondVertex(), aPoint.SecondVertex()) << "element " << anElement;
  }
}

TEST(AxisAnchorPointTest, IndicesAreOriginalAfterCleanup) {
  // Input 1 duplicates input 0 and is merged away, so cleaned vertex 1 is input 2.
  const Polygon aPolygon(
    std::vector<Point2d>{Point2d(0, 0), Point2d(0, 0), Point2d(2, 0), Point2d(2, 2), Point2d(0, 2)});

  const AxisAnchorPoint aVertex(aPolygon, 2);
  EXPECT_EQ(aVertex.Point(), Point2d(2, 0));
  EXPECT_EQ(aVertex.FirstVertex(), 2u);

  const AxisAnchorPoint anEdge(aPolygon, 1);
  EXPECT_EQ(anEdge.Point(), Point2d(1, 0));
  EXPECT_EQ(anEdge.FirstVertex(), 0u);
  EXPECT_EQ(anEdge.SecondVertex(), 2u);
}
