#pragma once

#include "psf/BaseTraits.hxx"
#include "psf/Point2d.hxx"
#include "psf/Polygon.hxx"
#include "psf/PolygonError.hxx"
#include "psf/Vector2d.hxx"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <random>
#include <vector>


//! Test support: random convex polygons that are τ-symmetric about a known axis.
namespace psf_test {

//! Random vector of length theLength * [0, 1], or exactly theLength when isOnBoundary.
inline psf::Vector2d RandomShift(std::mt19937& theGenerator, double theLength, bool isOnBoundary) {
  const double anAngle = std::uniform_real_distribution<double>(0.0, 2 * M_PI)(theGenerator);
  const double aScale  = isOnBoundary ? 1.0 : std::sqrt(std::uniform_real_distribution<double>(0.0, 1.0)(theGenerator));
  return {theLength * aScale * std::cos(anAngle), theLength * aScale * std::sin(anAngle)};
}

//! Applies a rotation by theAngle and then theTranslation.
inline psf::Point2d Move(const psf::Point2d& thePoint, double theAngle, const psf::Vector2d& theTranslation) {
  const double aX = thePoint.X() * std::cos(theAngle) - thePoint.Y() * std::sin(theAngle);
  const double aY = thePoint.X() * std::sin(theAngle) + thePoint.Y() * std::cos(theAngle);
  return psf::Point2d(aX, aY) + theTranslation;
}

struct TauSymmetricCase {
  std::vector<psf::Point2d> myPoints;
  double                    myTolerance = 0.0;
};

//! A convex polygon symmetric about the line through vertex 0: vertex 0 at angle 0, k vertices above, optionally one
//! at angle pi, and the k mirrored ones below, on a random ellipse. It is then made merely τ-symmetric (every vertex
//! moved by up to τ/2, or only the upper ones by up to τ, sometimes by the full amount) and moved by a random rigid
//! motion. The symmetry axis is centre 0 of the symbol string. τ is 1e-9 .. 1e-2 of the size, chosen by theIteration.
inline TauSymmetricCase MakeTauSymmetricCase(std::mt19937& theGenerator, int theIteration) {
  std::uniform_real_distribution<double> aUnit(0.0, 1.0);
  const double aRelativeTolerances[] = {1e-9, 1e-6, 1e-3, 1e-2};

  const std::size_t anUpperCount = std::uniform_int_distribution<std::size_t>(1, 8)(theGenerator);
  const bool hasOpposite  = aUnit(theGenerator) < 0.5;
  const double aScale     = std::pow(10.0, std::uniform_real_distribution<double>(-1.0, 2.0)(theGenerator));
  const double aSemiAxisX = aScale * (0.3 + aUnit(theGenerator));
  const double aSemiAxisY = aScale * (0.3 + aUnit(theGenerator));

  TauSymmetricCase aCase;
  aCase.myTolerance = aScale * aRelativeTolerances[theIteration % 4];

  std::vector<double> anUpperAngles(anUpperCount);
  for (double& anAngle : anUpperAngles) {
    anAngle = M_PI * (0.02 + 0.96 * aUnit(theGenerator));
  }
  std::sort(anUpperAngles.begin(), anUpperAngles.end());

  std::vector<double> anAngles{0.0};
  anAngles.insert(anAngles.end(), anUpperAngles.begin(), anUpperAngles.end());
  if (hasOpposite) {
    anAngles.push_back(M_PI);
  }
  for (auto anIt = anUpperAngles.rbegin(); anIt != anUpperAngles.rend(); ++anIt) {
    anAngles.push_back(2 * M_PI - *anIt);
  }

  // 0.999 leaves room for the rounding of the rigid motion.
  const bool isUpperOnly  = aUnit(theGenerator) < 0.5;
  const bool isOnBoundary = aUnit(theGenerator) < 0.5;
  const double aRotation  = 2 * M_PI * aUnit(theGenerator);
  const psf::Vector2d aTranslation{1000 * (aUnit(theGenerator) - 0.5), 1000 * (aUnit(theGenerator) - 0.5)};

  for (const double anAngle : anAngles) {
    psf::Point2d aPoint(aSemiAxisX * std::cos(anAngle), aSemiAxisY * std::sin(anAngle));
    const bool isUpper = anAngle > 0.0 && anAngle < M_PI;
    if (!isUpperOnly) {
      aPoint += RandomShift(theGenerator, 0.999 * aCase.myTolerance / 2, isOnBoundary);
    } else if (isUpper) {
      aPoint += RandomShift(theGenerator, 0.999 * aCase.myTolerance, isOnBoundary);
    }
    aCase.myPoints.push_back(Move(aPoint, aRotation, aTranslation));
  }
  return aCase;
}

//! The Polygon of thePoints with collinearity threshold min(τ, BaseTraits::Precision), or nothing if cleanup rejects
//! the points or drops a vertex (which would shift the indices a test expects). This test-only threshold is stricter
//! than the executable's r = τ on purpose: it keeps every vertex index stable, while r = τ would discard many cases.
inline std::optional<psf::Polygon> TryExactPolygon(const std::vector<psf::Point2d>& thePoints, double theTolerance) {
  try {
    psf::Polygon aPolygon(thePoints, std::min(theTolerance, psf::BaseTraits::Precision));
    if (aPolygon.Size() != thePoints.size()) {
      return std::nullopt;
    }
    return aPolygon;
  } catch (const psf::PolygonError&) {
    return std::nullopt;
  }
}

}
