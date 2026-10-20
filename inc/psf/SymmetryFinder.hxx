#pragma once

#include "psf/BaseTraits.hxx"
#include "psf/MirrorAxis.hxx"
#include "psf/Polygon.hxx"

#include <cstddef>
#include <vector>


namespace psf {

//! Finds the mirror axes of a convex polygon.
//! 1. Candidates: exact cyclic palindromes of the symbol string; they never miss a τ-symmetric axis.
//! 2. Group shortcut, O(N): the K candidates are checked together as one dihedral group (axes through the vertex
//!    centroid, angles from a least-squares fit). Mirror pairs compose into rotations, which the check uses
//!    internally; only mirror axes are reported. A pass proves every axis.
//! 3. Fallback, O(N) per axis: each candidate is checked against the line through its two axis anchor points.
//!    Verified if every reflected vertex lands within τ of its partner; dropped (rejected) only if some vertex
//!    misses by more than τ plus that vertex's provable estimation margin; Ambiguous otherwise.
//! Cost: O(N) when the group shortcut passes or K is bounded, O(N * K) in the worst case.
class SymmetryFinder {


public:

  using value_type = BaseTraits::float_type;

  SymmetryFinder() = delete;

  //! Returns the Verified and Ambiguous mirror axes of thePolygon; rejected candidates are not returned.
  //! theTolerance is τ; it must be finite and not negative, and it is not checked here.
  //! Verified axis - axis that reflects points in the Precision neighbourhood of opposite points
  //! Ambiguous axis - axis that reflects points in the slightly greater than Precision neighbourhood 
  static std::vector<MirrorAxis> Find(const Polygon& thePolygon, value_type theTolerance = BaseTraits::Precision);

};


}
