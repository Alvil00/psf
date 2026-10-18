#pragma once

#include "psf/Coords2d.hxx"

#include <cmath>


namespace psf {

class Vector2d : public Coords2d<Vector2d> {


public:

  //! Constructs the zero vector.
  constexpr Vector2d() noexcept = default;

  constexpr Vector2d(value_type theX, value_type theY) noexcept
  : Coords2d(theX, theY) {}

  constexpr value_type Dot(const Vector2d& theOther) const noexcept {
    return X() * theOther.X() + Y() * theOther.Y();
  }

  //! Returns the z-component of the 3D cross product.
  constexpr value_type Cross(const Vector2d& theOther) const noexcept {
    return X() * theOther.Y() - Y() * theOther.X();
  }

  constexpr value_type SquareMagnitude() const noexcept {
    return Dot(*this);
  }

  value_type Magnitude() const noexcept {
    return std::sqrt(SquareMagnitude());
  }

  constexpr bool IsZero(value_type theTolerance = BaseTraits::Precision) const noexcept {
    return SquareMagnitude() <= theTolerance * theTolerance;
  }

  //! Scales to unit length; a vector within theTolerance of zero becomes the zero vector.
  void Normalize(value_type theTolerance = BaseTraits::Precision) noexcept {
    if (IsZero(theTolerance)) {
      myCoords = {};
      return;
    }
    *this *= 1.0 / Magnitude();
  }

  //! Returns a unit-length copy; a vector within theTolerance of zero yields the zero vector.
  Vector2d Normalized(value_type theTolerance = BaseTraits::Precision) const noexcept {
    Vector2d aResult = *this;
    aResult.Normalize(theTolerance);
    return aResult;
  }

  //! Returns the vector rotated by 90 degrees counter-clockwise.
  constexpr Vector2d Perpendicular() const noexcept {
    return {-Y(), X()};
  }

  constexpr Vector2d& operator+=(const Vector2d& theOther) noexcept {
    myCoords[0] += theOther.X();
    myCoords[1] += theOther.Y();
    return *this;
  }

  constexpr Vector2d& operator-=(const Vector2d& theOther) noexcept {
    myCoords[0] -= theOther.X();
    myCoords[1] -= theOther.Y();
    return *this;
  }

  constexpr Vector2d& operator*=(value_type theScalar) noexcept {
    myCoords[0] *= theScalar;
    myCoords[1] *= theScalar;
    return *this;
  }

  friend constexpr Vector2d operator+(Vector2d theLeft, const Vector2d& theRight) noexcept {
    return theLeft += theRight;
  }

  friend constexpr Vector2d operator-(Vector2d theLeft, const Vector2d& theRight) noexcept {
    return theLeft -= theRight;
  }

  friend constexpr Vector2d operator-(const Vector2d& theVector) noexcept {
    return {-theVector.X(), -theVector.Y()};
  }

  friend constexpr Vector2d operator*(Vector2d theVector, value_type theScalar) noexcept {
    return theVector *= theScalar;
  }

  friend constexpr Vector2d operator*(value_type theScalar, Vector2d theVector) noexcept {
    return theVector *= theScalar;
  }

};


}
