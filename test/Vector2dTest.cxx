#include "psf/Vector2d.hxx"

#include <gtest/gtest.h>

#include <sstream>
#include <type_traits>
#include <utility>


namespace {

using psf::BaseTraits;
using psf::Vector2d;

template <typename TLeft, typename TRight, typename = void>
struct CanDivide : std::false_type {};

template <typename TLeft, typename TRight>
struct CanDivide<TLeft, TRight, std::void_t<decltype(std::declval<TLeft>() / std::declval<TRight>())>>
: std::true_type {};

static_assert(!CanDivide<Vector2d, double>::value, "Vector2d must not support division");
static_assert(!std::is_default_constructible_v<psf::Coords2d<Vector2d>>, "Coords2d must not be constructible directly");

}


TEST(Vector2dTest, DefaultIsZero) {
  const Vector2d aV;
  EXPECT_EQ(aV.X(), 0.0);
  EXPECT_EQ(aV.Y(), 0.0);
  EXPECT_TRUE(aV.IsZero());
}

TEST(Vector2dTest, ConstructsFromCoordinates) {
  const Vector2d aV{1.5, -2.5};
  EXPECT_EQ(aV.X(), 1.5);
  EXPECT_EQ(aV.Y(), -2.5);
}

TEST(Vector2dTest, EqualityIsExact) {
  EXPECT_EQ(Vector2d(1, 2), Vector2d(1, 2));
  EXPECT_NE(Vector2d(1, 2), Vector2d(1, 2 + 1e-12));
}

TEST(Vector2dTest, IsNearUsesEuclideanDistance) {
  const Vector2d aV{1, 1};
  EXPECT_TRUE(aV.IsNear(Vector2d(1 + 0.6e-9, 1 + 0.6e-9)));   // distance ~0.85e-9
  EXPECT_FALSE(aV.IsNear(Vector2d(1 + 0.8e-9, 1 + 0.8e-9)));  // distance ~1.13e-9
  EXPECT_TRUE(aV.IsNear(Vector2d(1.1, 1), 0.2));
}

TEST(Vector2dTest, AddsAndSubtracts) {
  const Vector2d aA{1, 2};
  const Vector2d aB{3, -5};
  EXPECT_EQ(aA + aB, Vector2d(4, -3));
  EXPECT_EQ(aA - aB, Vector2d(-2, 7));
  EXPECT_EQ(-aA, Vector2d(-1, -2));
}

TEST(Vector2dTest, CompoundOperatorsModifyInPlace) {
  Vector2d aV{1, 2};
  aV += Vector2d(1, 1);
  EXPECT_EQ(aV, Vector2d(2, 3));
  aV -= Vector2d(2, 0);
  EXPECT_EQ(aV, Vector2d(0, 3));
  aV *= 2.0;
  EXPECT_EQ(aV, Vector2d(0, 6));
}

TEST(Vector2dTest, MultipliesByScalarOnBothSides) {
  const Vector2d aV{1, -2};
  EXPECT_EQ(aV * 3.0, Vector2d(3, -6));
  EXPECT_EQ(3.0 * aV, Vector2d(3, -6));
}

TEST(Vector2dTest, DotProduct) {
  EXPECT_EQ(Vector2d(1, 2).Dot(Vector2d(3, 4)), 11.0);
  EXPECT_EQ(Vector2d(1, 0).Dot(Vector2d(0, 1)), 0.0);
}

TEST(Vector2dTest, CrossProductIsSignedArea) {
  EXPECT_EQ(Vector2d(1, 0).Cross(Vector2d(0, 1)), 1.0);
  EXPECT_EQ(Vector2d(0, 1).Cross(Vector2d(1, 0)), -1.0);
  EXPECT_EQ(Vector2d(2, 4).Cross(Vector2d(1, 2)), 0.0);
}

TEST(Vector2dTest, Magnitude) {
  const Vector2d aV{3, 4};
  EXPECT_EQ(aV.SquareMagnitude(), 25.0);
  EXPECT_DOUBLE_EQ(aV.Magnitude(), 5.0);
}

TEST(Vector2dTest, IsZeroRespectsTolerance) {
  EXPECT_TRUE(Vector2d(1e-10, 0).IsZero());
  EXPECT_FALSE(Vector2d(1e-8, 0).IsZero());
  EXPECT_TRUE(Vector2d(0.3, 0.4).IsZero(0.51));
  EXPECT_FALSE(Vector2d(0.3, 0.4).IsZero(0.49));
}

TEST(Vector2dTest, NormalizedReturnsUnitCopy) {
  const Vector2d aV{3, 4};
  const Vector2d aN = aV.Normalized();
  EXPECT_TRUE(aN.IsNear(Vector2d(0.6, 0.8)));
  EXPECT_DOUBLE_EQ(aN.Magnitude(), 1.0);
  EXPECT_EQ(aV, Vector2d(3, 4));
}

TEST(Vector2dTest, NormalizeModifiesInPlace) {
  Vector2d aV{0, -7};
  aV.Normalize();
  EXPECT_TRUE(aV.IsNear(Vector2d(0, -1)));
}

TEST(Vector2dTest, NearZeroNormalizesToZero) {
  Vector2d aV{1e-12, -1e-12};
  EXPECT_EQ(aV.Normalized(), Vector2d());
  aV.Normalize();
  EXPECT_EQ(aV, Vector2d());
}

TEST(Vector2dTest, NormalizeUsesGivenTolerance) {
  EXPECT_EQ(Vector2d(0.1, 0).Normalized(0.5), Vector2d());
  EXPECT_TRUE(Vector2d(0.1, 0).Normalized().IsNear(Vector2d(1, 0)));
}

TEST(Vector2dTest, PerpendicularRotatesCounterClockwise) {
  EXPECT_EQ(Vector2d(1, 0).Perpendicular(), Vector2d(0, 1));
  EXPECT_EQ(Vector2d(0, 1).Perpendicular(), Vector2d(-1, 0));
  const Vector2d aV{2, 3};
  EXPECT_EQ(aV.Dot(aV.Perpendicular()), 0.0);
  EXPECT_GT(aV.Cross(aV.Perpendicular()), 0.0);
}

TEST(Vector2dTest, IsUsableInConstantExpressions) {
  constexpr Vector2d aV = Vector2d(1, 2) + 2.0 * Vector2d(1, 1);
  static_assert(aV == Vector2d(3, 4), "constexpr arithmetic");
  static_assert(aV.SquareMagnitude() == 25.0, "constexpr magnitude");
  SUCCEED();
}

TEST(Vector2dTest, PrintsCoordinates) {
  std::ostringstream aStream;
  aStream << Vector2d(1.5, -2);
  EXPECT_EQ(aStream.str(), "(1.5, -2)");
}
