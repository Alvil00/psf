#include "psf/DihedralGroupElement.hxx"

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <vector>


namespace {

using psf::DihedralGroupElement;
using psf::Matrix2d;
using psf::Point2d;
using psf::Polygon;

//! The square (0, 0), (2, 0), (2, 2), (0, 2), counter-clockwise about its centre (1, 1).
Polygon Square() {
  return Polygon(std::vector<Point2d>{Point2d(0, 0), Point2d(2, 0), Point2d(2, 2), Point2d(0, 2)});
}

//! The square with vertex 2 pushed from (2, 2) to (2.5, 2).
Polygon PerturbedSquare() {
  return Polygon(std::vector<Point2d>{Point2d(0, 0), Point2d(2, 0), Point2d(2.5, 2), Point2d(0, 2)});
}

const Point2d theSquareCenter(1, 1);

}


TEST(DihedralGroupElementTest, DefaultIsTheNeutralElement) {
  const DihedralGroupElement anElement;
  EXPECT_EQ(anElement.Matrix(), Matrix2d::Identity());
  EXPECT_EQ(anElement.Shift(), 0u);
  EXPECT_TRUE(anElement.IsCyclicSubgroupElement());
  EXPECT_EQ(anElement.Center(), Point2d(0, 0));
  EXPECT_EQ(anElement.Apply(Point2d(3, -4)), Point2d(3, -4));
  for (std::size_t i = 0; i < 5; ++i) {
    EXPECT_EQ(anElement.MapIndex(i, 5), i);
  }
}

TEST(DihedralGroupElementTest, PartsAreKeptAsGiven) {
  const Matrix2d aMatrix(1, 2, 3, 4);
  const DihedralGroupElement anElement(aMatrix, 3, false, Point2d(0.5, -1));
  EXPECT_EQ(anElement.Matrix(), aMatrix);
  EXPECT_EQ(anElement.Shift(), 3u);
  EXPECT_FALSE(anElement.IsCyclicSubgroupElement());
  EXPECT_EQ(anElement.Center(), Point2d(0.5, -1));
}

TEST(DihedralGroupElementTest, MirrorPartnerReflectsIndicesAboutTheCenter) {
  // N = 4: the mirror centred on element 2 (vertex 1) fixes vertices 1 and 3 and swaps 0 and 2.
  EXPECT_EQ(DihedralGroupElement::MirrorPartner(2, 0, 4), 2u);
  EXPECT_EQ(DihedralGroupElement::MirrorPartner(2, 1, 4), 1u);
  EXPECT_EQ(DihedralGroupElement::MirrorPartner(2, 2, 4), 0u);
  EXPECT_EQ(DihedralGroupElement::MirrorPartner(2, 3, 4), 3u);
}

TEST(DihedralGroupElementTest, RotationShiftsIndices) {
  const DihedralGroupElement aRotation(Matrix2d::Rotation(M_PI / 2), 1, true, theSquareCenter);
  EXPECT_EQ(aRotation.MapIndex(0, 4), 1u);
  EXPECT_EQ(aRotation.MapIndex(3, 4), 0u);
}

TEST(DihedralGroupElementTest, ReflectionMapsIndicesToMirrorPartners) {
  const DihedralGroupElement aReflection(Matrix2d::Reflection(M_PI / 4), 2, false, theSquareCenter);
  for (std::size_t i = 0; i < 4; ++i) {
    EXPECT_EQ(aReflection.MapIndex(i, 4), DihedralGroupElement::MirrorPartner(2, i, 4));
  }
}

TEST(DihedralGroupElementTest, ApplyMovesAboutTheCenter) {
  const DihedralGroupElement aRotation(Matrix2d::Rotation(M_PI / 2), 1, true, theSquareCenter);
  EXPECT_TRUE(aRotation.Apply(Point2d(0, 0)).IsNear(Point2d(2, 0)));
  EXPECT_TRUE(aRotation.Apply(theSquareCenter).IsNear(theSquareCenter));

  const DihedralGroupElement aReflection(Matrix2d::Reflection(0.0), 0, false, theSquareCenter);
  EXPECT_TRUE(aReflection.Apply(Point2d(0, 0)).IsNear(Point2d(0, 2)));
}

TEST(DihedralGroupElementTest, DeviationVanishesForATrueSymmetry) {
  const Polygon aSquare = Square();
  // Quarter turn: vertex i goes to vertex i + 1. Mirror across the diagonal through vertices 0 and 2 (element 0).
  const DihedralGroupElement aRotation(Matrix2d::Rotation(M_PI / 2), 1, true, theSquareCenter);
  const DihedralGroupElement aReflection(Matrix2d::Reflection(M_PI / 4), 0, false, theSquareCenter);
  for (std::size_t i = 0; i < aSquare.Size(); ++i) {
    EXPECT_NEAR(aRotation.Deviation(aSquare, i), 0.0, 1e-12);
    EXPECT_NEAR(aReflection.Deviation(aSquare, i), 0.0, 1e-12);
  }
}

TEST(DihedralGroupElementTest, DeviationMeasuresAMissedPartner) {
  const Polygon aSquare = PerturbedSquare();
  // The quarter turn sends vertex 1 (2, 0) to (2, 2), but its index map points at the moved vertex 2 (2.5, 2).
  const DihedralGroupElement aRotation(Matrix2d::Rotation(M_PI / 2), 1, true, theSquareCenter);
  EXPECT_NEAR(aRotation.Deviation(aSquare, 1), 0.5, 1e-12);
  EXPECT_NEAR(aRotation.Deviation(aSquare, 0), 0.0, 1e-12);
}

TEST(DihedralGroupElementTest, DeviationReportsTheMappedIndex) {
  const Polygon aSquare = PerturbedSquare();
  const DihedralGroupElement aRotation(Matrix2d::Rotation(M_PI / 2), 1, true, theSquareCenter);
  const DihedralGroupElement aReflection(Matrix2d::Reflection(M_PI / 4), 0, false, theSquareCenter);
  for (const auto& anElement : {aRotation, aReflection}) {
    for (std::size_t i = 0; i < aSquare.Size(); ++i) {
      std::size_t anImage = 0;
      EXPECT_EQ(anElement.Deviation(aSquare, i, anImage), anElement.Deviation(aSquare, i));
      EXPECT_EQ(anImage, anElement.MapIndex(i, aSquare.Size()));
    }
  }
}
