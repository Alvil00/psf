#pragma once

#include "psf/BaseTraits.hxx"
#include "psf/Polygon.hxx"

#include <cstddef>
#include <vector>


namespace psf {

//! One element of the symbol string, compared by exact equality.
//! A vertex symbol holds the compatibility classes of (dot, cross) of the vertex's two edges;
//! an edge symbol holds the class of the edge length and 0.
struct Symbol {
  std::size_t myFirst  = 0;  //!< dot compatibility class for a vertex, length compatibility class for an edge
  std::size_t mySecond = 0;  //!< cross class for a vertex, 0 for an edge

  Symbol(std::size_t theFirst, std::size_t theSecond) noexcept : myFirst(theFirst), mySecond(theSecond) {}

  // reflexive, symmtric, transitive (the primary property for the Manacher) equal compare
  friend constexpr bool operator==(const Symbol& theLeft, const Symbol& theRight) noexcept {
    return theLeft.myFirst == theRight.myFirst && theLeft.mySecond == theRight.mySecond;
  }

  friend constexpr bool operator!=(const Symbol& theLeft, const Symbol& theRight) noexcept {
    return !(theLeft == theRight);
  }
};

//! Builds the symbol string of a clean polygon: vertex and edge symbols interleaved, 2N elements.
//! Element 2i is vertex i; element 2i + 1 is edge i, from vertex i to vertex i + 1 (mod N).
//! A cyclic palindrome centred on element c is a candidate axis through vertex c / 2 (c even)
//! or through the midpoint of edge (c - 1) / 2 (c odd).
//!
//! Guarantee: if the polygon is τ-symmetric about an axis, the symbol string is an exact cyclic palindrome
//! about that axis's centre. The class intervals are wide enough for this, including floating-point rounding.
class SymbolString {

public:

  using value_type = BaseTraits::float_type;

  SymbolString() = delete;

  //! theTolerance is τ, the largest distance allowed between a reflected vertex and its partner.
  //! theTolerance must be finite and not negative; it is not checked here.
  //! Throws PolygonError (TooLarge) if a vertex's dot or cross, or an edge length, is not finite.
  static std::vector<Symbol> Build(const Polygon& thePolygon, value_type theTolerance = BaseTraits::Precision);

};


}
