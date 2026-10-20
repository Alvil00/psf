#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>


namespace psf {

//! Error raised by Polygon when the cleaned-up points do not form a convex polygon, and by SymbolString when
//! the coordinates are so large that its arithmetic overflows.
class PolygonError : public std::runtime_error {


public:

  enum class ErrorKind {
    TooFewVertices,  //!< fewer than 3 vertices remain after merging duplicates and removing collinear vertices
    NotConvex,       //!< turns go both ways (or a vertex folds back onto its edge): VertexIndex() names one
    NotSimple,       //!< every turn goes the same way but the boundary winds around more than once, like a pentagram
    TooLarge         //!< a dot, cross or edge length overflows to inf or nan: VertexIndex() names the vertex
  };

  //! VertexIndex() for errors that are not tied to one vertex.
  static constexpr std::size_t NoVertex = std::numeric_limits<std::size_t>::max();

  //! theWhat is the complete message returned by what().
  PolygonError(ErrorKind theKind, std::size_t theVertexIndex, const std::string& theWhat)
  : std::runtime_error(theWhat), myKind(theKind), myVertexIndex(theVertexIndex) {}

  //! Kind of error
  ErrorKind Kind() const noexcept {
    return myKind;
  }

  //! Index into the input points of the offending vertex; NoVertex if the error is not tied to one.
  std::size_t VertexIndex() const noexcept {
    return myVertexIndex;
  }

private:

  ErrorKind   myKind;
  std::size_t myVertexIndex;

};


}
