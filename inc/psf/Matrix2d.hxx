#pragma once

#include "psf/BaseTraits.hxx"
#include "psf/Vector2d.hxx"

#include <array>
#include <cmath>
#include <cstddef>
#include <ostream>


namespace psf {

//! 2x2 matrix [[XX, XY], [YX, YY]] acting on column vectors: M * v.
//! It acts on Vector2d only; a point is moved about a centre c as c + M * (p - c).
class Matrix2d {


public:

  using value_type = BaseTraits::float_type;

  //! Constructs the zero matrix.
  constexpr Matrix2d() noexcept = default;

  constexpr Matrix2d(value_type theXX, value_type theXY, value_type theYX, value_type theYY) noexcept
  : myValues{theXX, theXY, theYX, theYY} {}

  static constexpr Matrix2d Identity() noexcept {
    return {1.0, 0.0, 0.0, 1.0};
  }

  //! Returns theLeft * theRight^T, the matrix whose (r, c) entry is theLeft[r] * theRight[c].
  static constexpr Matrix2d Outer(const Vector2d& theLeft, const Vector2d& theRight) noexcept {
    return {theLeft.X() * theRight.X(), theLeft.X() * theRight.Y(), theLeft.Y() * theRight.X(),
            theLeft.Y() * theRight.Y()};
  }

  //! Counter-clockwise rotation by theAngle radians.
  static Matrix2d Rotation(value_type theAngle) noexcept {
    const value_type aCos = std::cos(theAngle);
    const value_type aSin = std::sin(theAngle);
    return {aCos, -aSin, aSin, aCos};
  }

  //! Reflection across the line through the origin at theAxisAngle radians from the x-axis.
  static Matrix2d Reflection(value_type theAxisAngle) noexcept {
    const value_type aCos = std::cos(2 * theAxisAngle);
    const value_type aSin = std::sin(2 * theAxisAngle);
    return {aCos, aSin, aSin, -aCos};
  }

  //! Reflection across the line through the origin along theUnitDirection, which must have unit length.
  static constexpr Matrix2d Reflection(const Vector2d& theUnitDirection) noexcept {
    const value_type aX = theUnitDirection.X();
    const value_type aY = theUnitDirection.Y();
    const value_type anXY = 2 * aX * aY;
    return {aX * aX - aY * aY, anXY, anXY, aY * aY - aX * aX};
  }

  constexpr value_type XX() const noexcept {
    return myValues[0];
  }

  constexpr value_type XY() const noexcept {
    return myValues[1];
  }

  constexpr value_type YX() const noexcept {
    return myValues[2];
  }

  constexpr value_type YY() const noexcept {
    return myValues[3];
  }

  constexpr Matrix2d Transposed() const noexcept {
    return {XX(), YX(), XY(), YY()};
  }

  //! Returns true if the Frobenius norm of the difference to theOther does not exceed theTolerance.
  constexpr bool IsNear(const Matrix2d& theOther, value_type theTolerance = BaseTraits::Precision) const noexcept {
    value_type aSum = 0.0;
    for (std::size_t i = 0; i < myValues.size(); ++i) {
      const value_type aDelta = myValues[i] - theOther.myValues[i];
      aSum += aDelta * aDelta;
    }
    return aSum <= theTolerance * theTolerance;
  }

  constexpr Matrix2d& operator+=(const Matrix2d& theOther) noexcept {
    for (std::size_t i = 0; i < myValues.size(); ++i) {
      myValues[i] += theOther.myValues[i];
    }
    return *this;
  }

  constexpr Matrix2d& operator*=(value_type theScalar) noexcept {
    for (auto& aValue : myValues) {
      aValue *= theScalar;
    }
    return *this;
  }

  //! Right-multiplies: *this becomes *this * theOther.
  constexpr Matrix2d& operator*=(const Matrix2d& theOther) noexcept {
    return *this = *this * theOther;
  }

  friend constexpr Matrix2d operator+(Matrix2d theLeft, const Matrix2d& theRight) noexcept {
    return theLeft += theRight;
  }

  friend constexpr Matrix2d operator*(Matrix2d theMatrix, value_type theScalar) noexcept {
    return theMatrix *= theScalar;
  }

  friend constexpr Matrix2d operator*(value_type theScalar, Matrix2d theMatrix) noexcept {
    return theMatrix *= theScalar;
  }

  friend constexpr Vector2d operator*(const Matrix2d& theMatrix, const Vector2d& theVector) noexcept {
    return {theMatrix.XX() * theVector.X() + theMatrix.XY() * theVector.Y(),
            theMatrix.YX() * theVector.X() + theMatrix.YY() * theVector.Y()};
  }

  friend constexpr Matrix2d operator*(const Matrix2d& theLeft, const Matrix2d& theRight) noexcept {
    return {theLeft.XX() * theRight.XX() + theLeft.XY() * theRight.YX(),
            theLeft.XX() * theRight.XY() + theLeft.XY() * theRight.YY(),
            theLeft.YX() * theRight.XX() + theLeft.YY() * theRight.YX(),
            theLeft.YX() * theRight.XY() + theLeft.YY() * theRight.YY()};
  }

  //! Exact comparison; use IsNear for computed values.
  friend constexpr bool operator==(const Matrix2d& theLeft, const Matrix2d& theRight) noexcept {
    return theLeft.XX() == theRight.XX() && theLeft.XY() == theRight.XY() && theLeft.YX() == theRight.YX()
        && theLeft.YY() == theRight.YY();
  }

  friend constexpr bool operator!=(const Matrix2d& theLeft, const Matrix2d& theRight) noexcept {
    return !(theLeft == theRight);
  }

  friend std::ostream& operator<<(std::ostream& theStream, const Matrix2d& theMatrix) {
    return theStream << "((" << theMatrix.XX() << ", " << theMatrix.XY() << "), (" << theMatrix.YX() << ", "
                     << theMatrix.YY() << "))";
  }

private:

  std::array<value_type, 4> myValues{};  //!< row by row: XX, XY, YX, YY

};


}
