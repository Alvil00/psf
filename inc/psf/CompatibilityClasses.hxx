#pragma once

#include "psf/BaseTraits.hxx"

#include <cstddef>
#include <vector>


namespace psf {

//! Groups uncertain scalar values into compatibility classes.
//! Value i stands for the closed interval [value - halfWidth, value + halfWidth]. Two values whose intervals
//! overlap (touching counts) always get the same class; overlap is not transitive, so whole chains of
//! overlapping intervals merge into one class. Class ids are therefore an exact, transitive equality that
//! never separates two values that could be equal.
class CompatibilityClasses {


public:

  using value_type = BaseTraits::float_type;

  CompatibilityClasses() = delete;

  //! Returns the class id of every value. Ids are dense (1 .. K) and ascend with the intervals' lower bounds.
  //! Throws std::invalid_argument if the sizes differ. Every value and half-width must be finite and every
  //! half-width non-negative; otherwise the behaviour is undefined. These preconditions are not checked.
  static std::vector<std::size_t> Build(const std::vector<value_type>& theValues,
                                        const std::vector<value_type>& theHalfWidths);

};


}
