#include "psf/Polygon.hxx"

#include "psf/DebugOutput.hxx"
#include "psf/Point2d.hxx"
#include "psf/Vector2d.hxx"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>
#include <string>
#include <utility>


namespace psf {

namespace {

using value_type = Polygon::value_type;
using ErrorKind  = PolygonError::ErrorKind;

// Drop flags for EraseMarked, kept in std::uint8_t because std::vector<bool> is slow to index.
constexpr std::uint8_t theKept    = 0;
constexpr std::uint8_t theDropped = 1;

//! Distance from thePoint to the segment from theStart to theEnd; the distance to theStart if they coincide.
//! A point beyond either end measures to that end: a spike that doubles back along the line is far from the
//! segment, and nearly coincident ends cannot let rounding noise pick a line that happens to pass through it.
value_type DistanceToSegment(const Point2d& thePoint, const Point2d& theStart, const Point2d& theEnd) {
  const Vector2d aDirection = theEnd - theStart;
  const Vector2d anOffset   = thePoint - theStart;
  const value_type aSquareLength = aDirection.SquareMagnitude();
  if (aSquareLength == 0.0) {
    return anOffset.Magnitude();
  }
  const value_type aParam = anOffset.Dot(aDirection) / aSquareLength;
  if (aParam <= 0.0) {
    return anOffset.Magnitude();
  }
  if (aParam >= 1.0) {
    return (thePoint - theEnd).Magnitude();
  }
  return std::abs(aDirection.Cross(anOffset)) / std::sqrt(aSquareLength);
}

[[noreturn]] void ThrowTooFew(std::size_t theCount) {
  throw PolygonError(ErrorKind::TooFewVertices, PolygonError::NoVertex,
                     "polygon has " + std::to_string(theCount) + " vertices after cleanup, at least 3 are needed");
}

}


//! Step 1: drops every point within theCollinearity of the previously kept point, cyclically.
void Polygon::MergeDuplicates(value_type theCollinearity) {
  const auto aSize = myVertices.size();
  std::vector<std::uint8_t> isDropped(aSize, theKept);
  std::size_t aLastKept = 0;
  for (std::size_t i = 1; i < aSize; ++i) {
    if (myVertices[i].IsNear(myVertices[aLastKept], theCollinearity)) {
      isDropped[i] = theDropped;
    } else {
      aLastKept = i;
    }
  }
  // Closing duplicates: the cycle continues from the last kept point to the first one, which is always kept.
  for (std::size_t i = aLastKept; i > 0; --i) {
    if (isDropped[i] == theDropped) {
      continue;
    }
    if (!myVertices[i].IsNear(myVertices.front(), theCollinearity)) {
      break;
    }
    isDropped[i] = theDropped;
  }
  EraseMarked(isDropped);
}

//! Step 2: drops every vertex within theCollinearity of the segment between its neighbours.
//! All vertices are judged against the same neighbours, so the result does not depend on the walking direction.
void Polygon::RemoveCollinear(value_type theCollinearity) {
  const auto aSize = myVertices.size();
  std::vector<std::uint8_t> isDropped(aSize, theKept);
  for (std::size_t i = 0; i < aSize; ++i) {
    const value_type aDistance = DistanceToSegment(Vertex(i), Vertex(i + aSize - 1), Vertex(i + 1));
    if (aDistance > theCollinearity) {
      continue;
    }
    isDropped[i] = theDropped;
#if PSF_DEBUG_OUTPUT_ENABLED
    DebugOutput::Line("cleanup") << "dropped collinear input " << myOriginalIndices[i] << ' '
                                 << DebugOutput::Point(myVertices[i]) << ", distance "
                                 << DebugOutput::Number(aDistance) << '\n';
#endif
  }
  EraseMarked(isDropped);
}

//! Twice the signed area gives the orientation that most of the boundary follows, so a dent reported by CheckConvex
//! is a real one rather than whichever vertex happens to come first.
void Polygon::ComputeOrientation() {
  const auto aSize = myVertices.size();
  value_type aDoubleArea = 0.0;
  // Relative to the first vertex, which keeps the products small for polygons far from the origin.
  const Point2d& anOrigin = myVertices.front();
  for (std::size_t i = 1; i + 1 < aSize; ++i) {
    aDoubleArea += (myVertices[i] - anOrigin).Cross(myVertices[i + 1] - anOrigin);
  }
  myIsCounterClockwise = !(aDoubleArea < 0.0);
}

//! Step 3: every turn strictly in the orientation of the signed area, and one revolution in total.
void Polygon::CheckConvex() const {
  const auto aSize = myVertices.size();
  const value_type anOrientation = myIsCounterClockwise ? 1.0 : -1.0;

  value_type aTotalTurn = 0.0;
  for (std::size_t i = 0; i < aSize; ++i) {
    const Vector2d anIncoming = EdgeVector(i + aSize - 1);
    const Vector2d anOutgoing = EdgeVector(i);
    const value_type aCross = anIncoming.Cross(anOutgoing);
    if (aCross * anOrientation <= 0.0) {
      throw PolygonError(ErrorKind::NotConvex, myOriginalIndices[i],
                         "polygon is not convex at input vertex " + std::to_string(myOriginalIndices[i]));
    }
    aTotalTurn += std::atan2(aCross, anIncoming.Dot(anOutgoing));
  }

  // With every turn the same way, the total is exactly 2*pi times the winding count; 3*pi separates one
  // revolution from two with a wide margin for rounding.
  if (std::abs(aTotalTurn) > 3 * M_PI) {
    throw PolygonError(ErrorKind::NotSimple, PolygonError::NoVertex,
                       "polygon winds around more than once, so its edges cross");
  }
#if PSF_DEBUG_OUTPUT_ENABLED
  DebugOutput::Line("cleanup") << "convex, " << (anOrientation > 0.0 ? "counter-clockwise" : "clockwise")
                               << ", total turn " << DebugOutput::Number(aTotalTurn) << '\n';
#endif
}

//! The remove/erase idiom over both vectors at once: kept vertices move forward in step, the tail is erased.
void Polygon::EraseMarked(const std::vector<std::uint8_t>& theIsDropped) {
  std::size_t aKept = 0;
  for (std::size_t i = 0; i < myVertices.size(); ++i) {
    if (theIsDropped[i] == theDropped) {
      continue;
    }
    myVertices[aKept]     = myVertices[i];
    myOriginalIndices[aKept] = myOriginalIndices[i];
    ++aKept;
  }
  myVertices.erase(myVertices.begin() + aKept, myVertices.end());
  myOriginalIndices.erase(myOriginalIndices.begin() + aKept, myOriginalIndices.end());
}

//! Edge i is vertex i + 1 minus vertex i; the closing edge runs from the last vertex back to the first.
void Polygon::BuildEdges() {
  myEdges.reserve(myVertices.size());
  std::transform(myVertices.begin() + 1, myVertices.end(), myVertices.begin(), std::back_inserter(myEdges),
                 [](const Point2d& theTo, const Point2d& theFrom) { return theTo - theFrom; });
  myEdges.push_back(myVertices.front() - myVertices.back());
}

//! Points cannot be added, so this sums their offsets from the origin and averages those.
Point2d Polygon::VertexCentroid() const {
  const Vector2d aSum = std::accumulate(myVertices.cbegin(), myVertices.cend(), Vector2d{},
                                        [](const Vector2d& theSum, const Point2d& theVertex) {
                                          return theSum + (theVertex - Point2d::Origin());
                                        });
  return Point2d::Origin() + aSum * (1.0 / static_cast<value_type>(myVertices.size()));
}


Polygon::Polygon(std::vector<Point2d>&& thePoints, value_type theCollinearity) {
  myVertices = std::move(thePoints);
  myOriginalIndices.resize(myVertices.size());
  std::iota(myOriginalIndices.begin(), myOriginalIndices.end(), std::size_t{0});

#if PSF_DEBUG_OUTPUT_ENABLED
  const auto anInputCount = myVertices.size();
#endif
  MergeDuplicates(theCollinearity);
#if PSF_DEBUG_OUTPUT_ENABLED
  DebugOutput::Line("cleanup") << myVertices.size() << " of " << anInputCount
                               << " points left after merging duplicates, r = " << DebugOutput::Number(theCollinearity)
                               << '\n';
  for (std::size_t i = 0; i < myVertices.size(); ++i) {
    DebugOutput::Line("cleanup") << "kept input " << myOriginalIndices[i] << ' ' << DebugOutput::Point(myVertices[i])
                                 << '\n';
  }
#endif
  if (myVertices.size() < 3) {
    ThrowTooFew(myVertices.size());
  }

  RemoveCollinear(theCollinearity);
  if (myVertices.size() < 3) {
    ThrowTooFew(myVertices.size());
  }

  BuildEdges();
  ComputeOrientation();
  CheckConvex();

#if PSF_DEBUG_OUTPUT_ENABLED
  DebugOutput::Line("polygon") << myVertices.size() << " vertices\n";
  for (std::size_t i = 0; i < myVertices.size(); ++i) {
    DebugOutput::Line("polygon") << "vertex " << i << " = input " << myOriginalIndices[i] << ' '
                                 << DebugOutput::Point(myVertices[i]) << '\n';
  }
#endif
}

}
