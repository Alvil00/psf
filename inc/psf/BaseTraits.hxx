#pragma once

namespace psf {

//! Some basic preferences for programm
struct BaseTraits {

  // Float point precision used for programm thing
  using float_type = double;

  //! Default absolute tolerance for geometric comparisons.
  static constexpr float_type Precision = 1e-9;

};

}
