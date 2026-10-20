#pragma once

#include "psf/BaseTraits.hxx"
#include "psf/Point2d.hxx"
#include "psf/PolygonError.hxx"
#include "psf/Vector2d.hxx"

#include <cstddef>
#include <cstdint>
#include <vector>


namespace psf {

//! A clean convex polygon: at least 3 vertices in boundary order, closed implicitly, every turn strictly in the
//! same direction, winding around once. Either orientation is accepted.
//! The constructor cleans the input points and throws if what remains is not such a polygon,
//! so every Polygon object is clean and convex.
class Polygon {


public:

  using value_type = BaseTraits::float_type;

  //! Cleans thePoints in this order:
  //! 1. Merges duplicates: walking the cycle once, including from the last point back to the first, a point within
  //!    theCollinearity of the previously kept point is dropped.
  //! 2. Removes collinear vertices in one pass: a vertex within theCollinearity of the segment between its two
  //!    neighbours (after step 1) is dropped. Removal is not re-checked against the new neighbours.
  //! 3. Checks that at least 3 vertices remain, that every turn goes the same way, and that the turns add up to
  //!    one full revolution.
  //! theCollinearity is r: it should not exceed the symmetry tolerance τ, so cleanup moves the outline by at most τ.
  //! theCollinearity must be finite and not negative; it is not checked here.
  //! Throws PolygonError if a check fails. Arithmetic overflow from huge coordinates is not detected here and
  //! may be reported as NotConvex; SymbolString detects it.
  explicit Polygon(std::vector<Point2d>&& thePoints, value_type theCollinearity = BaseTraits::Precision);

  //! Copies thePoints and cleans the copy, as the constructor above.
  explicit Polygon(const std::vector<Point2d>& thePoints, value_type theCollinearity = BaseTraits::Precision)
  : Polygon(std::vector<Point2d>(thePoints), theCollinearity) {}

  //! Number of vertices after cleanup
  std::size_t Size() const noexcept {
    return myVertices.size();
  }

  //! Vertices in boundary order.
  const std::vector<Point2d>& Vertices() const noexcept {
    return myVertices;
  }

  //! Vertex theIndex in boundary order, taken modulo Size(), so theIndex may run past the last vertex.
  const Point2d& Vertex(std::size_t theIndex) const noexcept {
    if (theIndex < myVertices.size()) {
      return myVertices[theIndex];
    }
    return myVertices[theIndex % myVertices.size()];
  }

  //! Edge vectors in boundary order, one per vertex: edge i runs from vertex i to vertex i + 1, and the last one
  //! closes the boundary from the last vertex back to the first. Computed once by the constructor.
  const std::vector<Vector2d>& Edges() const noexcept {
    return myEdges;
  }

  //! Edge theIndex as a vector, from vertex theIndex to vertex theIndex + 1, taken modulo Size().
  const Vector2d& EdgeVector(std::size_t theIndex) const noexcept {
    if (theIndex < myEdges.size()) {
      return myEdges[theIndex];
    }
    return myEdges[theIndex % myEdges.size()];
  }

  //! For every vertex, its original index: its position in the points given to the constructor.
  const std::vector<std::size_t>& OriginalIndices() const noexcept {
    return myOriginalIndices;
  }

  //! Arithmetic mean of the vertices, not the area centroid. Every symmetry fixes both; this one is cheaper and is
  //! the centre the least-squares mirror fit in SymmetryFinder works around.
  Point2d VertexCentroid() const;

  //! True if the boundary runs counter-clockwise, false if clockwise. Computed once by the constructor.
  bool IsCounterClockwise() const noexcept {
    return myIsCounterClockwise;
  }

private:

  //! Cleanup step 1 on myVertices and myOriginalIndices; see the constructor.
  void MergeDuplicates(value_type theCollinearity);

  //! Cleanup step 2 on myVertices and myOriginalIndices; see the constructor.
  void RemoveCollinear(value_type theCollinearity);

  //! Sets myIsCounterClockwise from the sign of the area of the cleaned myVertices.
  void ComputeOrientation();

  //! Cleanup step 3, the convexity part: throws PolygonError unless every turn goes the same way and the turns add
  //! up to one full revolution.
  void CheckConvex() const;

  //! Removes every vertex whose flag in theIsDropped is set, from myVertices and myOriginalIndices alike, keeping the
  //! order of the rest. theIsDropped has one flag per vertex.
  void EraseMarked(const std::vector<std::uint8_t>& theIsDropped);

  //! Fills myEdges from the cleaned myVertices.
  void BuildEdges();

  std::vector<Point2d>     myVertices;
  std::vector<std::size_t> myOriginalIndices;
  std::vector<Vector2d>    myEdges;
  bool                     myIsCounterClockwise = true;

};


}
