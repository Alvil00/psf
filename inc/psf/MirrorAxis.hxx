#pragma once

#include "psf/AxisAnchorPoint.hxx"
#include "psf/BaseTraits.hxx"
#include "psf/Polygon.hxx"

#include <cstddef>


namespace psf {

//! How well a reported mirror axis is established.
enum class AxisStatus {
  Verified,  //!< proven: reflecting across the axis puts every vertex within τ of its partner
  Ambiguous  //!< neither proven nor disproven: the measured deviation lies inside the estimation margin
             //! Margin depends on of the indefenity of axis because of precison
};

//! A reported mirror axis: the line through its two axis anchor points.
class MirrorAxis {


public:

  using value_type = BaseTraits::float_type;

  MirrorAxis() noexcept = default;

  //! Takes the parts as given; nothing is checked.
  MirrorAxis(const AxisAnchorPoint& theFirst, const AxisAnchorPoint& theSecond, AxisStatus theStatus,
             value_type theDeviation) noexcept
  : myFirst(theFirst),
    mySecond(theSecond),
    myStatus(theStatus),
    myDeviation(theDeviation) {}

  //! The axis of thePolygon centred on symbol element theCenter: anchored at elements theCenter and theCenter + N.
  MirrorAxis(const Polygon& thePolygon, std::size_t theCenter, AxisStatus theStatus, value_type theDeviation);

  //! First anchor point of the axis
  const AxisAnchorPoint& First() const noexcept {
    return myFirst;
  }

  //! Second anchor point of the axis
  const AxisAnchorPoint& Second() const noexcept {
    return mySecond;
  }

  //! Status of the axis that shows the deviation level
  AxisStatus Status() const noexcept {
    return myStatus;
  }

  //! Largest measured distance between a reflected vertex and its partner.
  value_type Deviation() const noexcept {
    return myDeviation;
  }

private:

  AxisAnchorPoint myFirst;
  AxisAnchorPoint mySecond;
  AxisStatus      myStatus    = AxisStatus::Ambiguous;
  value_type      myDeviation = 0.0;
};


}
