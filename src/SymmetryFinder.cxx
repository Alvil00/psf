#include "psf/SymmetryFinder.hxx"

#include "psf/AxisAnchorPoint.hxx"
#include "psf/CyclicPalindromes.hxx"
#include "psf/DebugOutput.hxx"
#include "psf/DihedralGroupElement.hxx"
#include "psf/Matrix2d.hxx"
#include "psf/SymbolString.hxx"
#include "psf/Vector2d.hxx"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>


namespace psf {

namespace {

using value_type = SymmetryFinder::value_type;

//! Unit roundoff of value_type: the largest relative error of one rounded operation.
constexpr value_type theUnitRoundoff = std::numeric_limits<value_type>::epsilon() / 2;

#if PSF_DEBUG_OUTPUT_ENABLED
//! Symbol element theElement as "vertex i = input j (x, y)" or "edge i-i+1 = inputs j-k, midpoint (x, y)".
std::string DescribeElement(const Polygon& thePolygon, std::size_t theElement) {
  const auto aSize = thePolygon.Size();
  const std::size_t anElement = theElement % (2 * aSize);
  const std::size_t aFirst    = anElement / 2;
  const AxisAnchorPoint aPoint(thePolygon, anElement);
  if (aPoint.IsVertex()) {
    return "vertex " + std::to_string(aFirst) + " = input " + std::to_string(aPoint.FirstVertex()) + ' '
         + DebugOutput::Point(aPoint.Point());
  }
  return "edge " + std::to_string(aFirst) + '-' + std::to_string((aFirst + 1) % aSize) + " = inputs "
       + std::to_string(aPoint.FirstVertex()) + '-' + std::to_string(aPoint.SecondVertex()) + ", midpoint "
       + DebugOutput::Point(aPoint.Point());
}
#endif

//! Least-squares mirror centred on symbol element theCenter about theCentroid (orthogonal Procrustes): with
//! z = v - centroid and p = z_partner, minimising sum |R z_j - p_j|^2 over reflections R maximises sum p_j . R z_j,
//! the entrywise product of R with the cross-covariance C = sum p_j z_j^T. For R = [[cos 2t, sin 2t],
//! [sin 2t, -cos 2t]] that is cos 2t (C_xx - C_yy) + sin 2t (C_xy + C_yx), largest at
//! 2t = atan2(C_xy + C_yx, C_xx - C_yy).
Matrix2d CalculateBaseMirror(const Polygon& thePolygon, std::size_t theCenter, const Point2d& theCentroid) {
  const auto& aVertices = thePolygon.Vertices();
  const auto aSize = thePolygon.Size();

  Matrix2d aCovariance;
  for (std::size_t j = 0; j < aSize; ++j) {
    const Vector2d aZ       = aVertices[j] - theCentroid;
    const Vector2d aPartner = aVertices[DihedralGroupElement::MirrorPartner(theCenter, j, aSize)] - theCentroid;
    aCovariance += Matrix2d::Outer(aPartner, aZ);
  }
  const value_type aFitCos = aCovariance.XX() - aCovariance.YY();
  const value_type aFitSin = aCovariance.XY() + aCovariance.YX();
  return Matrix2d::Reflection(0.5 * std::atan2(aFitSin, aFitCos));
}

//! Group shortcut. The candidates are centres c_m = c_0 + m * s (s = N / K); their mirrors and the rotations they
//! compose into form a dihedral group of 2K index maps. The matching isometries are fixed by the vertex centroid,
//! the orientation and a least-squares angle for the mirror at c_0 (CalculateBaseMirror):
//!   rotation k:   i -> i + k s,  angle  o * 2 pi k / K
//!   reflection k: i -> c_k - i,  axis   theta_0 + o * pi k / K       (o = +1 counter-clockwise, -1 clockwise)
//! These compose exactly like their index maps, whatever the estimate. Every vertex j of an orbit is g(r) for some
//! element g and the orbit's first vertex r, and so is its partner under any mirror R: (R g)(r). Hence
//!   |R v_j - v_R(j)| <= |v_j - g v_r| + |v_R(j) - (R g) v_r| <= 2 * (largest |v_g(r) - g v_r| over the orbit).
//! The argument holds for any centre and any theta_0, so the fit only tightens the bound; a degenerate fit
//! (atan2(0, 0) = 0) still yields a valid, if loose, one. Returns that bound for all axes at once. O(N): each orbit
//! has at least K vertices and is walked once with 2K elements.
value_type DihedralGroupDeviationBound(const Polygon& thePolygon, const std::vector<std::size_t>& theCenters) {
  const auto aSize  = thePolygon.Size();
  const auto aCount = theCenters.size();
  const auto aStep  = aSize / aCount;

  const Point2d    aCentroid     = thePolygon.VertexCentroid();
  const Matrix2d   aBaseMirror   = CalculateBaseMirror(thePolygon, theCenters.front(), aCentroid);
  const value_type anOrientation = thePolygon.IsCounterClockwise() ? 1.0 : -1.0;
  const auto       aCountValue   = static_cast<value_type>(aCount);

  std::vector<DihedralGroupElement> anElements;
  anElements.reserve(2 * aCount);
  for (std::size_t k = 0; k < aCount; ++k) {
    const auto anIndex   = static_cast<value_type>(k);
    const auto aRotation = Matrix2d::Rotation(anOrientation * 2 * M_PI * anIndex / aCountValue);
    anElements.emplace_back(aRotation, (k * aStep) % aSize, true, aCentroid);

    // Rotating by phi after mirroring across theta_0 mirrors across theta_0 + phi / 2, the axis of reflection k.
    anElements.emplace_back(aRotation * aBaseMirror, (theCenters.front() + k * aStep) % aSize, false, aCentroid);
  }

  // I always avoid the usage of std::vector<bool> because of big performance issues (especially on MSVC)
  constexpr std::uint8_t aFalse = 0;
  constexpr std::uint8_t aTrue = 1;
  std::vector<std::uint8_t> isVisited(aSize, aFalse);
  value_type aLargest = 0.0;
  for (std::size_t aRoot = 0; aRoot < aSize; ++aRoot) {
    if (isVisited[aRoot] == aTrue) {
      continue;
    }
    for (const auto& anElement : anElements) {
      std::size_t anImage = 0;
      aLargest = std::max(aLargest, anElement.Deviation(thePolygon, aRoot, anImage));
      isVisited[anImage] = aTrue;
    }
  }
  return 2 * aLargest;
}

//! Fallback: checks the mirror centred on symbol element theCenter against the line through its anchor points A, B.
//! If the polygon is τ-symmetric about some true axis L, both anchor points lie within τ/2 of L (a vertex on the axis
//! moves by at most τ, and so does the midpoint of an edge whose endpoints swap). So the estimated line is off by at
//! most (τ/2)(|t| + |1 - t|) at chord parameter t and by an angle e <= asin(τ / |AB|), and reflecting a vertex at
//! signed distance d from it differs from reflecting across L by at most
//!   margin = 2 * ((τ/2)(|t| + |1 - t|) + |d| (e + 1 - cos e)).
//! The candidate is rejected only if some vertex misses its partner by more than τ + margin + rounding. Otherwise
//! appends the axis through A, B with its status and largest deviation to theResult.
void CheckAndInsertAxis(const Polygon& thePolygon, std::size_t theCenter, value_type theTolerance,
                        value_type theRounding, std::vector<MirrorAxis>& theResult) {
  const auto& aVertices = thePolygon.Vertices();
  const auto aSize = thePolygon.Size();

  const AxisAnchorPoint aFirst(thePolygon, theCenter);
  const AxisAnchorPoint aSecond(thePolygon, theCenter + aSize);
  const Point2d& aStart = aFirst.Point();
  const Vector2d aChord = aSecond.Point() - aStart;
  const value_type aChordLength = aChord.Magnitude();

  const Vector2d aDirection = aChord * (1.0 / aChordLength);
  const Vector2d aNormal    = aDirection.Perpendicular();
  const Matrix2d aMirror    = Matrix2d::Reflection(aDirection);
  const value_type anAngle  = theTolerance >= aChordLength ? M_PI / 2 : std::asin(theTolerance / aChordLength);
  const value_type anAngleTerm = anAngle + (1.0 - std::cos(anAngle));

  value_type aLargest = 0.0;
  for (std::size_t j = 0; j < aSize; ++j) {
    const Vector2d anOffset  = aVertices[j] - aStart;
    const value_type aParam  = anOffset.Dot(aDirection) / aChordLength;
    const value_type aSigned = anOffset.Dot(aNormal);
    const Point2d aReflected = aStart + aMirror * anOffset;
    const value_type aDeviation =
      (aReflected - aVertices[DihedralGroupElement::MirrorPartner(theCenter, j, aSize)]).Magnitude();
    const value_type aMargin =
      2 * (0.5 * theTolerance * (std::abs(aParam) + std::abs(1.0 - aParam)) + std::abs(aSigned) * anAngleTerm);

    aLargest = std::max(aLargest, aDeviation);
    if (aDeviation > theTolerance + aMargin + theRounding) {
#if PSF_DEBUG_OUTPUT_ENABLED
      DebugOutput::Line("fallback") << "centre " << theCenter << ", deviation " << DebugOutput::Number(aDeviation)
                                    << " at vertex " << j << ", rejected\n";
#endif
      return;
    }
  }
  const auto aStatus = aLargest + theRounding <= theTolerance ? AxisStatus::Verified : AxisStatus::Ambiguous;
  theResult.emplace_back(aFirst, aSecond, aStatus, aLargest);
#if PSF_DEBUG_OUTPUT_ENABLED
  DebugOutput::Line("fallback") << "centre " << theCenter << ", deviation " << DebugOutput::Number(aLargest) << ", "
                                << (aStatus == AxisStatus::Verified ? "verified" : "ambiguous") << '\n';
#endif
}

}


std::vector<MirrorAxis> SymmetryFinder::Find(const Polygon& thePolygon, value_type theTolerance) {
  // Find palindromes by Manacher with transitive binary relatios of compatibility classes
  const auto aCenters = CyclicPalindromes::Centers(SymbolString::Build(thePolygon, theTolerance));

#if PSF_DEBUG_OUTPUT_ENABLED
  if (aCenters.empty()) {
    DebugOutput::Line("centres") << "none, so there are no candidate axes\n";
  } else {
    DebugOutput::Line("centres") << aCenters.size() << " candidates, step " << thePolygon.Size() / aCenters.size()
                                 << '\n';
  }
  for (const auto aCenter : aCenters) {
    DebugOutput::Line("centres") << "centre " << aCenter << ": " << DescribeElement(thePolygon, aCenter) << " to "
                                 << DescribeElement(thePolygon, aCenter + thePolygon.Size()) << '\n';
  }
#endif

  if (aCenters.empty()) {
    return {};
  }

  // Exact palindromes about c_1 and c_2 make the symbol string periodic with period 2 (c_2 - c_1), so the centres
  // always form one arithmetic progression with a step dividing N.
  const auto aSize  = thePolygon.Size();
  const auto aCount = aCenters.size();
  const auto aStep  = aSize / aCount;

#ifndef NDEBUG
  for (std::size_t m = 0; m < aCount; ++m) {
    if (aSize % aCount != 0 || aCenters[m] != aCenters.front() + m * aStep) {
      throw std::logic_error("SymmetryFinder: candidate centres are not evenly spaced");
    }
  }
#endif

  // Rounding of reflections, rotations and distances: a few units of roundoff times the coordinate magnitude.
  value_type aMagnitude = 0.0;
  for (const auto& aVertex : thePolygon.Vertices()) {
    aMagnitude = std::max({aMagnitude, std::abs(aVertex.X()), std::abs(aVertex.Y())});
  }
  const value_type aRounding = 64 * theUnitRoundoff * aMagnitude;

  std::vector<MirrorAxis> anAxes;
  if (aCount >= 2) {
    const value_type aBound = DihedralGroupDeviationBound(thePolygon, aCenters);
#if PSF_DEBUG_OUTPUT_ENABLED
    DebugOutput::Line("group") << "K = " << aCount << ", bound " << DebugOutput::Number(aBound) << ", tolerance "
                               << DebugOutput::Number(theTolerance) << ", rounding " << DebugOutput::Number(aRounding)
                               << ", " << (aBound + aRounding <= theTolerance ? "passed" : "failed") << '\n';
#endif
    if (aBound + aRounding <= theTolerance) {
      for (const auto aCenter : aCenters) {
        anAxes.emplace_back(thePolygon, aCenter, AxisStatus::Verified, aBound);
      }
      return anAxes;
    }
  }

#if PSF_DEBUG_OUTPUT_ENABLED
  if (aCount < 2) {
    DebugOutput::Line("group") << "skipped, only one candidate\n";
  }
#endif

  for (const auto aCenter : aCenters) {
    CheckAndInsertAxis(thePolygon, aCenter, theTolerance, aRounding, anAxes);
  }

  return anAxes;
}

}
