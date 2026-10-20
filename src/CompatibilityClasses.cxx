#include "psf/CompatibilityClasses.hxx"

#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <numeric>
#include <stdexcept>


namespace psf {

namespace {

using value_type = CompatibilityClasses::value_type;

//! Returns the indices of theKeys in ascending key order. The keys must be finite; ties come in unspecified order.
std::vector<std::size_t> SortedOrder(const std::vector<value_type>& theKeys) {
  std::vector<std::size_t> anOrder(theKeys.size());
  std::iota(anOrder.begin(), anOrder.end(), std::size_t{0});
  std::sort(anOrder.begin(), anOrder.end(),
            [&](std::size_t theLeft, std::size_t theRight) { return theKeys[theLeft] < theKeys[theRight]; });
  return anOrder;
}

}


std::vector<std::size_t> CompatibilityClasses::Build(const std::vector<value_type>& theValues,
                                                     const std::vector<value_type>& theHalfWidths) {
  if (theValues.size() != theHalfWidths.size()) {
    throw std::invalid_argument("CompatibilityClasses: values and half-widths differ in size");
  }
  const auto aSize = theValues.size();
  if (aSize == 0) {
    return {};
  }

  // Rounding to nearest is monotone, so if two intervals overlap exactly, their computed bounds still satisfy
  // lower <= upper; the bounds need no outward rounding.
  std::vector<value_type> aLower;
  aLower.reserve(aSize);
  std::transform(theValues.begin(), theValues.end(), theHalfWidths.begin(), std::back_inserter(aLower),
                 std::minus<>{});

  std::vector<value_type> anUpper;
  anUpper.reserve(aSize);
  std::transform(theValues.begin(), theValues.end(), theHalfWidths.begin(), std::back_inserter(anUpper),
                 std::plus<>{});

  // Sweep the intervals by lower bound: an interval starting after the current class's rightmost upper bound
  // overlaps none of them, nor any interval after it, so it opens a new class. Intervals with equal lower bounds
  // always overlap, so the order among them does not change the classes. The reach starts below every finite
  // bound, so the first interval opens class 1.
  std::vector<std::size_t> aClasses(aSize);
  std::size_t aClass = 0;
  value_type  aReach = -std::numeric_limits<value_type>::infinity();
  for (const auto anIndex : SortedOrder(aLower)) {
    if (aLower[anIndex] > aReach) {
      ++aClass;
    }
    aReach = std::max(aReach, anUpper[anIndex]);
    aClasses[anIndex] = aClass;
  }
  return aClasses;
}

}
