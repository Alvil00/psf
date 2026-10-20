#include "psf/Matrix2d.hxx"

#include <gtest/gtest.h>

#include <cmath>
#include <sstream>


namespace {

using psf::Matrix2d;
using psf::Vector2d;

constexpr double theTolerance = 1e-12;

}


TEST(Matrix2dTest, DefaultIsZero) {
  EXPECT_EQ(Matrix2d(), Matrix2d(0, 0, 0, 0));
}

TEST(Matrix2dTest, ConstructsRowByRow) {
  const Matrix2d aM(1, 2, 3, 4);
  EXPECT_EQ(aM.XX(), 1.0);
  EXPECT_EQ(aM.XY(), 2.0);
  EXPECT_EQ(aM.YX(), 3.0);
  EXPECT_EQ(aM.YY(), 4.0);
}

TEST(Matrix2dTest, EqualityIsExact) {
  EXPECT_EQ(Matrix2d(1, 2, 3, 4), Matrix2d(1, 2, 3, 4));
  EXPECT_NE(Matrix2d(1, 2, 3, 4), Matrix2d(1, 2, 3, 4 + 1e-15));
}

TEST(Matrix2dTest, IsNearUsesFrobeniusNorm) {
  const Matrix2d aM(1, 1, 1, 1);
  EXPECT_TRUE(aM.IsNear(Matrix2d(1.3, 1, 1, 1.4), 0.5));   // norm 0.5
  EXPECT_FALSE(aM.IsNear(Matrix2d(1.3, 1, 1, 1.5), 0.5));  // norm ~0.58
}

TEST(Matrix2dTest, IdentityKeepsVectorsAndMatrices) {
  const Matrix2d aM(1, 2, 3, 4);
  EXPECT_EQ(Matrix2d::Identity() * Vector2d(5, -6), Vector2d(5, -6));
  EXPECT_EQ(Matrix2d::Identity() * aM, aM);
  EXPECT_EQ(aM * Matrix2d::Identity(), aM);
}

TEST(Matrix2dTest, OuterProduct) {
  EXPECT_EQ(Matrix2d::Outer(Vector2d(1, 2), Vector2d(3, 4)), Matrix2d(3, 4, 6, 8));
}

TEST(Matrix2dTest, Adds) {
  Matrix2d aM(1, 2, 3, 4);
  EXPECT_EQ(aM + Matrix2d(10, 20, 30, 40), Matrix2d(11, 22, 33, 44));
  aM += Matrix2d(1, 1, 1, 1);
  EXPECT_EQ(aM, Matrix2d(2, 3, 4, 5));
}

TEST(Matrix2dTest, MultipliesByScalarOnBothSides) {
  const Matrix2d aM(1, 2, 3, 4);
  EXPECT_EQ(aM * 2.0, Matrix2d(2, 4, 6, 8));
  EXPECT_EQ(2.0 * aM, Matrix2d(2, 4, 6, 8));
  Matrix2d aCopy = aM;
  aCopy *= -1.0;
  EXPECT_EQ(aCopy, Matrix2d(-1, -2, -3, -4));
}

TEST(Matrix2dTest, MultipliesByVector) {
  EXPECT_EQ(Matrix2d(1, 2, 3, 4) * Vector2d(5, 6), Vector2d(17, 39));
}

TEST(Matrix2dTest, MultipliesByMatrixInOrder) {
  const Matrix2d aLeft(1, 2, 3, 4);
  const Matrix2d aRight(5, 6, 7, 8);
  EXPECT_EQ(aLeft * aRight, Matrix2d(19, 22, 43, 50));
  EXPECT_EQ(aRight * aLeft, Matrix2d(23, 34, 31, 46));
  Matrix2d aCopy = aLeft;
  aCopy *= aRight;
  EXPECT_EQ(aCopy, aLeft * aRight);
}

TEST(Matrix2dTest, Transposes) {
  EXPECT_EQ(Matrix2d(1, 2, 3, 4).Transposed(), Matrix2d(1, 3, 2, 4));
}

TEST(Matrix2dTest, RotationTurnsCounterClockwise) {
  EXPECT_TRUE((Matrix2d::Rotation(M_PI / 2) * Vector2d(1, 0)).IsNear(Vector2d(0, 1), theTolerance));
  EXPECT_TRUE((Matrix2d::Rotation(M_PI / 2) * Vector2d(0, 1)).IsNear(Vector2d(-1, 0), theTolerance));
}

TEST(Matrix2dTest, RotationsAddTheirAngles) {
  EXPECT_TRUE((Matrix2d::Rotation(0.3) * Matrix2d::Rotation(1.1)).IsNear(Matrix2d::Rotation(1.4), theTolerance));
}

TEST(Matrix2dTest, RotationTransposedIsTheInverseRotation) {
  EXPECT_TRUE(Matrix2d::Rotation(0.7).Transposed().IsNear(Matrix2d::Rotation(-0.7), theTolerance));
}

TEST(Matrix2dTest, ReflectionFixesTheAxisAndFlipsTheNormal) {
  const double anAngle = 0.4;
  const Vector2d anAxis(std::cos(anAngle), std::sin(anAngle));
  const auto aMirror = Matrix2d::Reflection(anAngle);
  EXPECT_TRUE((aMirror * anAxis).IsNear(anAxis, theTolerance));
  EXPECT_TRUE((aMirror * anAxis.Perpendicular()).IsNear(-anAxis.Perpendicular(), theTolerance));
}

TEST(Matrix2dTest, ReflectionIsAnInvolution) {
  const auto aMirror = Matrix2d::Reflection(1.2);
  EXPECT_TRUE((aMirror * aMirror).IsNear(Matrix2d::Identity(), theTolerance));
}

TEST(Matrix2dTest, ReflectionFromDirectionMatchesReflectionFromAngle) {
  const double anAngle = -2.3;
  EXPECT_TRUE(Matrix2d::Reflection(Vector2d(std::cos(anAngle), std::sin(anAngle)))
                .IsNear(Matrix2d::Reflection(anAngle), theTolerance));
}

TEST(Matrix2dTest, RotationAfterReflectionTurnsTheAxisByHalfTheAngle) {
  // The dihedral group relation that SymmetryFinder builds its mirrors from.
  const double anAxis = 0.25;
  const double aTurn  = 1.9;
  EXPECT_TRUE((Matrix2d::Rotation(aTurn) * Matrix2d::Reflection(anAxis))
                .IsNear(Matrix2d::Reflection(anAxis + aTurn / 2), theTolerance));
}

TEST(Matrix2dTest, IsUsableInConstantExpressions) {
  constexpr Matrix2d aProduct = Matrix2d(1, 2, 3, 4) * Matrix2d::Identity() + Matrix2d::Outer({1, 0}, {0, 1});
  static_assert(aProduct == Matrix2d(1, 3, 3, 4), "Matrix2d arithmetic must be constexpr");
  static_assert(Matrix2d::Reflection(Vector2d(0, 1)) == Matrix2d(-1, 0, 0, 1), "Reflection across the y-axis");
  SUCCEED();
}

TEST(Matrix2dTest, PrintsRowByRow) {
  std::ostringstream aStream;
  aStream << Matrix2d(1, 2, 3, 4);
  EXPECT_EQ(aStream.str(), "((1, 2), (3, 4))");
}
