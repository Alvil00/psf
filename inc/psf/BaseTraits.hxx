#pragma once

namespace psf {

struct BaseTraits {

  using float_type = double;

  //! Default absolute tolerance for geometric comparisons.
  static constexpr float_type Precision = 1e-9;

};

}
