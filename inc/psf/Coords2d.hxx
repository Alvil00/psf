#pragma once

#include "psf/BaseTraits.hxx"

#include <array>
#include <ostream>


namespace psf {

//! CRTP base holding the coordinates shared by Point2d and Vector2d.
//! Comparisons accept only the same derived type, so a point never compares with a vector.
template <typename TDerived>
class Coords2d {


public:

  using value_type = BaseTraits::float_type;

  constexpr value_type X() const noexcept {
    return myCoords[0];
  }

  constexpr value_type Y() const noexcept {
    return myCoords[1];
  }

  //! Returns true if the Euclidean distance to theOther does not exceed theTolerance.
  constexpr bool IsNear(const TDerived& theOther,
                        value_type theTolerance = BaseTraits::Precision) const noexcept {
    const value_type aDX = X() - theOther.X();
    const value_type aDY = Y() - theOther.Y();
    return aDX * aDX + aDY * aDY <= theTolerance * theTolerance;
  }

  //! Exact comparison; use IsNear for computed values.
  friend constexpr bool operator==(const TDerived& theLeft, const TDerived& theRight) noexcept {
    return theLeft.X() == theRight.X() && theLeft.Y() == theRight.Y();
  }

  friend constexpr bool operator!=(const TDerived& theLeft, const TDerived& theRight) noexcept {
    return !(theLeft == theRight);
  }

  friend std::ostream& operator<<(std::ostream& theStream, const TDerived& theCoords) {
    return theStream << '(' << theCoords.X() << ", " << theCoords.Y() << ')';
  }

protected:

  constexpr Coords2d() noexcept = default;

  constexpr Coords2d(value_type theX, value_type theY) noexcept
  : myCoords{theX, theY} {}

  std::array<value_type, 2> myCoords{};

};


}
