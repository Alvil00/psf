#include "psf/SymbolString.hxx"

#include "psf/CompatibilityClasses.hxx"
#include "psf/DebugOutput.hxx"
#include "psf/PolygonError.hxx"
#include "psf/Vector2d.hxx"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>


namespace psf {

namespace {

using value_type = SymbolString::value_type;

//! Unit roundoff of value_type: the largest relative error of one rounded operation.
constexpr value_type theUnitRoundoff = std::numeric_limits<value_type>::epsilon() / 2;

//! Relative slack on the computed geometric bounds, which are themselves rounded.
constexpr value_type theBoundSlack = 1 + 16 * theUnitRoundoff;

#if PSF_DEBUG_OUTPUT_ENABLED
//! Prints one feature's values, half-widths and compatibility classes; ids are dense from 1, so the count is the max.
void DebugClasses(std::string_view                theFeature,
                  const std::vector<value_type>&  theValues,
                  const std::vector<value_type>&  theHalfWidths,
                  const std::vector<std::size_t>& theClasses) {
  std::size_t aCount = 0;
  for (std::size_t i = 0; i < theValues.size(); ++i) {
    DebugOutput::Line("classes") << theFeature << ' ' << i << " value " << DebugOutput::Number(theValues[i])
                                 << ", half-width " << DebugOutput::Number(theHalfWidths[i]) << ", class "
                                 << theClasses[i] << '\n';
    aCount = std::max(aCount, theClasses[i]);
  }
  DebugOutput::Line("classes") << theFeature << ": " << aCount << " classes\n";
}
#endif

}


std::vector<Symbol> SymbolString::Build(const Polygon& thePolygon, value_type theTolerance) {
  const auto aSize = thePolygon.Size();

  const auto aTau = theTolerance;
  const auto aTauSquare = aTau * aTau;

  // Edge i runs from vertex i to vertex i + 1. A τ-symmetric partner edge moves each endpoint by at most τ,
  // so the lengths differ by at most 2τ: half-width τ each.
  // Rounding: two coordinate differences, two squares, a sum and a square root stay well within 4u * L.
  const auto& anEdges = thePolygon.Edges();
  std::vector<value_type> aLengths;
  aLengths.reserve(aSize);
  std::transform(anEdges.begin(), anEdges.end(), std::back_inserter(aLengths),
                 [](const Vector2d& theEdge) { return theEdge.Magnitude(); });

  std::vector<value_type> aLengthWidths;
  aLengthWidths.reserve(aSize);
  std::transform(aLengths.begin(), aLengths.end(), std::back_inserter(aLengthWidths), [aTau](value_type theLength) {
    return aTau * theBoundSlack + 4 * theUnitRoundoff * theLength;
  });

  // Vertex B = vertex i between A = vertex i - 1 and C = vertex i + 1, with edges a = B - A and b = C - B.
  // Moving A, B, C by at most τ each changes (B moves both edges, so its terms are grouped):
  //   cross(a, b) by at most τ (|a| + |b| + |a + b|)      + 3τ²
  //   dot(a, b)   by at most τ (|a| + |b| + |A - 2B + C|) + 4τ²
  // The partner's bound uses its own lengths; taking half of each side's bound as its half-width covers
  // min(bound_i, bound_j) <= (bound_i + bound_j) / 2.
  // Rounding: a difference, a product and a subtraction stay within 5u * (sum of |products|).
  std::vector<value_type> aDots; aDots.reserve(aSize);
  std::vector<value_type> aDotWidths; aDotWidths.reserve(aSize);
  std::vector<value_type> aCrosses; aCrosses.reserve(aSize);
  std::vector<value_type> aCrossWidths; aCrossWidths.reserve(aSize);

  // Process the dot and the cross for one polygon vertex. Vertex aVertex sits between edges A and B, and B leaves it,
  // so checking the length of B covers every edge once. Finite input coordinates can still overflow here.
  // Each call handles the next vertex, so aVertex advances by one per call.
  const auto& anOriginalIndices = thePolygon.OriginalIndices();
  std::size_t aVertex = 0;
  auto aCrossDotProcessor =
    [&aDots, &aDotWidths, &aCrosses, &aCrossWidths, &anOriginalIndices, &aVertex, aTau, aTauSquare](
      const Vector2d& theEdgeA,
      const Vector2d& theEdgeB,
      value_type      theLengthA,
      value_type      theLengthB) -> void {
    const auto aDot   = theEdgeA.Dot(theEdgeB);
    const auto aCross = theEdgeA.Cross(theEdgeB);
    if (!std::isfinite(aDot) || !std::isfinite(aCross) || !std::isfinite(theLengthB)) {
      const auto anIndex = anOriginalIndices[aVertex];
      throw PolygonError(PolygonError::ErrorKind::TooLarge, anIndex,
                         "coordinates too large: arithmetic overflows at input vertex " + std::to_string(anIndex));
    }

    const auto aLengthSum = theLengthA + theLengthB;
    const auto aCrossBound = aTau * (aLengthSum + (theEdgeB + theEdgeA).Magnitude()) + 3.0 * aTauSquare;
    const auto aDotBound   = aTau * (aLengthSum + (theEdgeB - theEdgeA).Magnitude()) + 4.0 * aTauSquare;

    const auto aDotRounding =
      5.0 * theUnitRoundoff * (std::abs(theEdgeA.X() * theEdgeB.X()) + std::abs(theEdgeA.Y() * theEdgeB.Y()));
    const auto aCrossRounding =
      5.0 * theUnitRoundoff * (std::abs(theEdgeA.X() * theEdgeB.Y()) + std::abs(theEdgeA.Y() * theEdgeB.X()));

    aDots.push_back(aDot);
    aDotWidths.push_back(0.5 * aDotBound * theBoundSlack + aDotRounding);
    aCrosses.push_back(aCross);
    aCrossWidths.push_back(0.5 * aCrossBound * theBoundSlack + aCrossRounding);

    ++aVertex;
  };

  // Process the seam pair
  aCrossDotProcessor(anEdges.back(), anEdges.front(), aLengths.back(), aLengths.front());

  // Process the rest edges
  auto anEdgeA = anEdges.cbegin(); auto anEdgeB = anEdges.cbegin() + 1;
  auto aLengthA = aLengths.cbegin(); auto aLengthB = aLengths.cbegin() + 1;
  for (; anEdgeB != anEdges.cend(); ++anEdgeA, ++anEdgeB, ++aLengthA, ++aLengthB) {
    aCrossDotProcessor(*anEdgeA, *anEdgeB, *aLengthA, *aLengthB);
  }

  // Each feature gets its own classes; reading a mirror image backwards leaves all three unchanged
  // (the sign flip of cross from reversing the order cancels the one from the reflection).
  const auto aLengthClasses = CompatibilityClasses::Build(aLengths, aLengthWidths);
  const auto aDotClasses    = CompatibilityClasses::Build(aDots, aDotWidths);
  const auto aCrossClasses  = CompatibilityClasses::Build(aCrosses, aCrossWidths);
#if PSF_DEBUG_OUTPUT_ENABLED
  DebugClasses("edge length", aLengths, aLengthWidths, aLengthClasses);
  DebugClasses("vertex dot", aDots, aDotWidths, aDotClasses);
  DebugClasses("vertex cross", aCrosses, aCrossWidths, aCrossClasses);
#endif

  std::vector<Symbol> aSymbols; 
  aSymbols.reserve(2 * aSize);
  auto aDotClass = aDotClasses.cbegin();
  auto aCrossClass = aCrossClasses.cbegin();
  auto aLengthClass = aLengthClasses.cbegin();
  // Create the symbol string from compatibility classes
  for (; aDotClass != aDotClasses.cend(); ++aDotClass, ++aCrossClass, ++aLengthClass) {
    aSymbols.emplace_back(*aDotClass, *aCrossClass); //< Vertex
    aSymbols.emplace_back(*aLengthClass, 0); //< Edge
  }
#if PSF_DEBUG_OUTPUT_ENABLED
  for (std::size_t i = 0; i < aSize; ++i) {
    DebugOutput::Line("symbols") << "element " << 2 * i << " vertex " << i << " (dot " << aDotClasses[i]
                                 << ", cross " << aCrossClasses[i] << ")\n";
    DebugOutput::Line("symbols") << "element " << 2 * i + 1 << " edge " << i << '-' << (i + 1) % aSize
                                 << " (length " << aLengthClasses[i] << ")\n";
  }
#endif
  return aSymbols;
}

}
