#include "psf/MirrorAxis.hxx"

#include <gtest/gtest.h>

#include <vector>


namespace {

using psf::AxisAnchorPoint;
using psf::AxisStatus;
using psf::MirrorAxis;
using psf::Point2d;
using psf::Polygon;

//! The square (0, 0), (2, 0), (2, 2), (0, 2): N = 4, so its symbol elements run from 0 to 7.
Polygon Square() {
  return Polygon(std::vector<Point2d>{Point2d(0, 0), Point2d(2, 0), Point2d(2, 2), Point2d(0, 2)});
}

}


TEST(MirrorAxisTest, DefaultIsAnAmbiguousPlaceholder) {
  const MirrorAxis anAxis;
  EXPECT_EQ(anAxis.First().Point(), Point2d(0, 0));
  EXPECT_TRUE(anAxis.First().IsVertex());
  EXPECT_EQ(anAxis.Second().Point(), Point2d(0, 0));
  EXPECT_TRUE(anAxis.Second().IsVertex());
  EXPECT_EQ(anAxis.Status(), AxisStatus::Ambiguous);
  EXPECT_EQ(anAxis.Deviation(), 0.0);
}

TEST(MirrorAxisTest, PartsAreKeptAsGiven) {
  const MirrorAxis anAxis(AxisAnchorPoint(Point2d(0.5, 0), 0, 1), AxisAnchorPoint(Point2d(-1, 3), 2, 2),
                          AxisStatus::Verified, 0.25);
  EXPECT_EQ(anAxis.First().Point(), Point2d(0.5, 0));
  EXPECT_EQ(anAxis.First().FirstVertex(), 0u);
  EXPECT_EQ(anAxis.First().SecondVertex(), 1u);
  EXPECT_EQ(anAxis.Second().Point(), Point2d(-1, 3));
  EXPECT_EQ(anAxis.Second().FirstVertex(), 2u);
  EXPECT_EQ(anAxis.Status(), AxisStatus::Verified);
  EXPECT_EQ(anAxis.Deviation(), 0.25);
}

TEST(MirrorAxisTest, VertexCenterRunsToTheOppositeVertex) {
  const MirrorAxis anAxis(Square(), 0, AxisStatus::Verified, 0.0);
  EXPECT_TRUE(anAxis.First().IsVertex());
  EXPECT_EQ(anAxis.First().FirstVertex(), 0u);
  EXPECT_EQ(anAxis.First().Point(), Point2d(0, 0));
  EXPECT_TRUE(anAxis.Second().IsVertex());
  EXPECT_EQ(anAxis.Second().FirstVertex(), 2u);
  EXPECT_EQ(anAxis.Second().Point(), Point2d(2, 2));
}

TEST(MirrorAxisTest, EdgeCenterRunsToTheOppositeEdge) {
  const MirrorAxis anAxis(Square(), 3, AxisStatus::Ambiguous, 0.5);
  EXPECT_TRUE(anAxis.First().IsEdgeCenter());
  EXPECT_EQ(anAxis.First().FirstVertex(), 1u);
  EXPECT_EQ(anAxis.First().SecondVertex(), 2u);
  EXPECT_EQ(anAxis.First().Point(), Point2d(2, 1));
  EXPECT_TRUE(anAxis.Second().IsEdgeCenter());
  EXPECT_EQ(anAxis.Second().FirstVertex(), 3u);
  EXPECT_EQ(anAxis.Second().SecondVertex(), 0u);
  EXPECT_EQ(anAxis.Second().Point(), Point2d(0, 1));
  EXPECT_EQ(anAxis.Status(), AxisStatus::Ambiguous);
  EXPECT_EQ(anAxis.Deviation(), 0.5);
}
