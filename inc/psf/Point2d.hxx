#pragma once

#include "psf/Coords2d.hxx"
#include "psf/Vector2d.hxx"

#include <cmath>


namespace psf {

//! Affine point: Point - Point gives a Vector2d, Point +/- Vector2d gives a Point2d.
//! Point + Point and scaling a point are intentionally not defined.
class Point2d : public Coords2d<Point2d> {


public:

  //! Constructs the origin.
  constexpr Point2d() noexcept = default;

  constexpr Point2d(value_type theX, value_type theY) noexcept
  : Coords2d(theX, theY) {}

  static constexpr Point2d Origin() noexcept {
    return {};
  }

  constexpr value_type SquareDistance(const Point2d& theOther) const noexcept {
    return (*this - theOther).SquareMagnitude();
  }

  value_type Distance(const Point2d& theOther) const noexcept {
    return std::sqrt(SquareDistance(theOther));
  }

  constexpr Point2d& operator+=(const Vector2d& theVector) noexcept {
    myCoords[0] += theVector.X();
    myCoords[1] += theVector.Y();
    return *this;
  }

  constexpr Point2d& operator-=(const Vector2d& theVector) noexcept {
    myCoords[0] -= theVector.X();
    myCoords[1] -= theVector.Y();
    return *this;
  }

  friend constexpr Vector2d operator-(const Point2d& theLeft, const Point2d& theRight) noexcept {
    return {theLeft.X() - theRight.X(), theLeft.Y() - theRight.Y()};
  }

  friend constexpr Point2d operator+(Point2d thePoint, const Vector2d& theVector) noexcept {
    return thePoint += theVector;
  }

  friend constexpr Point2d operator+(const Vector2d& theVector, Point2d thePoint) noexcept {
    return thePoint += theVector;
  }

  friend constexpr Point2d operator-(Point2d thePoint, const Vector2d& theVector) noexcept {
    return thePoint -= theVector;
  }

};


}
