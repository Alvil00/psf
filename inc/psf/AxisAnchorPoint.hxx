#pragma once

#include "psf/Point2d.hxx"
#include "psf/Polygon.hxx"

#include <cstddef>


namespace psf {

//! One end of a mirror axis on the polygon boundary: a vertex, or the midpoint of an edge.
class AxisAnchorPoint {


public:

  //! A placeholder for containers and tests, not a point of any polygon: vertex 0 at the origin.
  AxisAnchorPoint() noexcept = default;

  //! Takes the parts as given; nothing is checked. theSecondVertex equals theFirstVertex for a vertex.
  AxisAnchorPoint(const Point2d& thePoint, std::size_t theFirstVertex, std::size_t theSecondVertex) noexcept
  : myPoint(thePoint),
    myFirstVertex(theFirstVertex),
    mySecondVertex(theSecondVertex) {}

  //! The vertex (even element) or edge midpoint (odd element) at symbol element theElement of thePolygon.
  //! theElement is taken modulo 2N, so it may run past the last element. Vertex indices are original indices.
  AxisAnchorPoint(const Polygon& thePolygon, std::size_t theElement);

  const Point2d& Point() const noexcept {
    return myPoint;
  }

  //! Original index of the vertex, or of the edge's first vertex.
  std::size_t FirstVertex() const noexcept {
    return myFirstVertex;
  }

  //! Original index of the edge's second vertex; equals FirstVertex() for a vertex.
  std::size_t SecondVertex() const noexcept {
    return mySecondVertex;
  }

  bool IsVertex() const noexcept {
    return myFirstVertex == mySecondVertex;
  }

  bool IsEdgeCenter() const noexcept {
    return !IsVertex();
  }

private:

  Point2d     myPoint;
  std::size_t myFirstVertex  = 0;
  std::size_t mySecondVertex = 0;
};


}
