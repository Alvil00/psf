#include "psf/Point2d.hxx"

#include <gtest/gtest.h>

#include <sstream>
#include <type_traits>
#include <utility>


namespace {

using psf::Point2d;
using psf::Vector2d;

template <typename TLeft, typename TRight, typename = void>
struct CanAdd : std::false_type {};

template <typename TLeft, typename TRight>
struct CanAdd<TLeft, TRight, std::void_t<decltype(std::declval<TLeft>() + std::declval<TRight>())>>
: std::true_type {};

template <typename TLeft, typename TRight, typename = void>
struct CanMultiply : std::false_type {};

template <typename TLeft, typename TRight>
struct CanMultiply<TLeft, TRight, std::void_t<decltype(std::declval<TLeft>() * std::declval<TRight>())>>
: std::true_type {};

template <typename TLeft, typename TRight, typename = void>
struct CanCompare : std::false_type {};

template <typename TLeft, typename TRight>
struct CanCompare<TLeft, TRight, std::void_t<decltype(std::declval<TLeft>() == std::declval<TRight>())>>
: std::true_type {};

static_assert(!CanAdd<Point2d, Point2d>::value, "Point + Point must not compile");
static_assert(!CanMultiply<Point2d, double>::value, "Point * scalar must not compile");
static_assert(!CanMultiply<double, Point2d>::value, "scalar * Point must not compile");
static_assert(!CanCompare<Point2d, Vector2d>::value, "Point and Vector must not be comparable");
static_assert(!std::is_convertible_v<Point2d, Vector2d>, "no implicit Point -> Vector");
static_assert(!std::is_convertible_v<Vector2d, Point2d>, "no implicit Vector -> Point");

}


TEST(Point2dTest, DefaultIsOrigin) {
  const Point2d aP;
  EXPECT_EQ(aP.X(), 0.0);
  EXPECT_EQ(aP.Y(), 0.0);
  EXPECT_EQ(aP, Point2d::Origin());
}

TEST(Point2dTest, ConstructsFromCoordinates) {
  const Point2d aP{-1.5, 2.5};
  EXPECT_EQ(aP.X(), -1.5);
  EXPECT_EQ(aP.Y(), 2.5);
}

TEST(Point2dTest, EqualityIsExact) {
  EXPECT_EQ(Point2d(1, 2), Point2d(1, 2));
  EXPECT_NE(Point2d(1, 2), Point2d(1 + 1e-12, 2));
}

TEST(Point2dTest, IsNearUsesEuclideanDistance) {
  const Point2d aP{5, 5};
  EXPECT_TRUE(aP.IsNear(Point2d(5 + 0.6e-9, 5 - 0.6e-9)));   // distance ~0.85e-9
  EXPECT_FALSE(aP.IsNear(Point2d(5 + 0.8e-9, 5 - 0.8e-9)));  // distance ~1.13e-9
  EXPECT_TRUE(aP.IsNear(Point2d(5.3, 5.4), 0.51));
  EXPECT_FALSE(aP.IsNear(Point2d(5.3, 5.4), 0.49));
}

TEST(Point2dTest, DifferenceOfPointsIsVector) {
  const Vector2d aV = Point2d(4, 6) - Point2d(1, 2);
  EXPECT_EQ(aV, Vector2d(3, 4));
}

TEST(Point2dTest, TranslatesByVector) {
  const Point2d aP{1, 2};
  const Vector2d aV{3, -1};
  EXPECT_EQ(aP + aV, Point2d(4, 1));
  EXPECT_EQ(aV + aP, Point2d(4, 1));
  EXPECT_EQ(aP - aV, Point2d(-2, 3));
}

TEST(Point2dTest, CompoundTranslationModifiesInPlace) {
  Point2d aP{1, 2};
  aP += Vector2d(1, 1);
  EXPECT_EQ(aP, Point2d(2, 3));
  aP -= Vector2d(2, 3);
  EXPECT_EQ(aP, Point2d::Origin());
}

TEST(Point2dTest, RoundTripThroughOrigin) {
  const Point2d aP{7, -3};
  const Vector2d aPosition = aP - Point2d::Origin();
  EXPECT_EQ(aPosition, Vector2d(7, -3));
  EXPECT_EQ(Point2d::Origin() + aPosition, aP);
}

TEST(Point2dTest, Distance) {
  const Point2d aA{1, 2};
  const Point2d aB{4, 6};
  EXPECT_EQ(aA.SquareDistance(aB), 25.0);
  EXPECT_DOUBLE_EQ(aA.Distance(aB), 5.0);
  EXPECT_DOUBLE_EQ(aB.Distance(aA), 5.0);
  EXPECT_EQ(aA.Distance(aA), 0.0);
}

TEST(Point2dTest, IsUsableInConstantExpressions) {
  constexpr Point2d aP = Point2d(1, 2) + Vector2d(2, 2);
  static_assert(aP == Point2d(3, 4), "constexpr translation");
  static_assert(aP.SquareDistance(Point2d::Origin()) == 25.0, "constexpr distance");
  SUCCEED();
}

TEST(Point2dTest, PrintsCoordinates) {
  std::ostringstream aStream;
  aStream << Point2d(-1, 0.25);
  EXPECT_EQ(aStream.str(), "(-1, 0.25)");
}
