#include "psf/CompatibilityClasses.hxx"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>


namespace {

using psf::CompatibilityClasses;
using Classes = std::vector<std::size_t>;
using Values  = std::vector<double>;

//! Test oracle: connected components of the interval-overlap graph, O(N^2) with union-find.
//! Returns, for every element, the smallest index in its component.
Classes BruteForceComponents(const Values& theValues, const Values& theHalfWidths) {
  const auto aSize = theValues.size();
  Classes aParent(aSize);
  std::iota(aParent.begin(), aParent.end(), std::size_t{0});
  const auto aFind = [&](std::size_t theIndex) {
    while (aParent[theIndex] != theIndex) {
      theIndex = aParent[theIndex];
    }
    return theIndex;
  };
  for (std::size_t i = 0; i < aSize; ++i) {
    for (std::size_t j = i + 1; j < aSize; ++j) {
      const bool isOverlap = theValues[i] - theHalfWidths[i] <= theValues[j] + theHalfWidths[j]
                          && theValues[j] - theHalfWidths[j] <= theValues[i] + theHalfWidths[i];
      if (isOverlap) {
        const std::size_t aRootI = aFind(i);
        const std::size_t aRootJ = aFind(j);
        aParent[std::max(aRootI, aRootJ)] = std::min(aRootI, aRootJ);
      }
    }
  }
  Classes aRoots(aSize);
  for (std::size_t i = 0; i < aSize; ++i) {
    aRoots[i] = aFind(i);
  }
  return aRoots;
}

//! True if both labelings split the elements into the same groups, whatever the label values.
bool IsSamePartition(const Classes& theLeft, const Classes& theRight) {
  for (std::size_t i = 0; i < theLeft.size(); ++i) {
    for (std::size_t j = i + 1; j < theLeft.size(); ++j) {
      if ((theLeft[i] == theLeft[j]) != (theRight[i] == theRight[j])) {
        return false;
      }
    }
  }
  return true;
}

}


TEST(CompatibilityClassesTest, EmptyInputGivesNoClasses) {
  EXPECT_TRUE(CompatibilityClasses::Build({}, {}).empty());
}

TEST(CompatibilityClassesTest, RejectsMismatchedSizes) {
  EXPECT_THROW(CompatibilityClasses::Build({1.0}, {}), std::invalid_argument);
}

TEST(CompatibilityClassesTest, SingleValueGetsClassOne) {
  EXPECT_EQ(CompatibilityClasses::Build({-7.0}, {0.0}), (Classes{1}));
}

TEST(CompatibilityClassesTest, SeparatedValuesGetSeparateClasses) {
  EXPECT_EQ(CompatibilityClasses::Build({0.0, 0.2}, {0.05, 0.05}), (Classes{1, 2}));
}

TEST(CompatibilityClassesTest, ChainOfOverlapsMergesIntoOneClass) {
  // 0 and 0.18 are 0.18 apart, more than 2 * 0.05, but 0.09 overlaps both.
  EXPECT_EQ(CompatibilityClasses::Build({0.0, 0.09, 0.18}, {0.05, 0.05, 0.05}), (Classes{1, 1, 1}));
}

TEST(CompatibilityClassesTest, TouchingIntervalsShareAClass) {
  EXPECT_EQ(CompatibilityClasses::Build({0.0, 1.0}, {0.5, 0.5}), (Classes{1, 1}));
}

TEST(CompatibilityClassesTest, WideIntervalBridgesNarrowOnes) {
  EXPECT_EQ(CompatibilityClasses::Build({0.0, 10.0, 5.0}, {0.1, 0.1, 5.0}), (Classes{1, 1, 1}));
}

TEST(CompatibilityClassesTest, IdsAscendWithValueWhateverTheInputOrder) {
  EXPECT_EQ(CompatibilityClasses::Build({5.0, -3.0, 1.0}, {0.0, 0.0, 0.0}), (Classes{3, 1, 2}));
}

TEST(CompatibilityClassesTest, OrdersNegativesZerosAndPositives) {
  // Zero half-widths: only equal values share a class, and -0.0 equals +0.0.
  const Values aValues{3.0, 1e-300, -0.0, 0.0, -1e-300, -2.5, -std::numeric_limits<double>::max()};
  const Values aWidths(aValues.size(), 0.0);
  EXPECT_EQ(CompatibilityClasses::Build(aValues, aWidths), (Classes{6, 5, 4, 4, 3, 2, 1}));
}

TEST(CompatibilityClassesTest, MatchesBruteForceComponentsOnRandomIntervals) {
  std::mt19937 aGenerator(20261018);
  for (int anIter = 0; anIter < 2000; ++anIter) {
    const auto aSize = std::uniform_int_distribution<std::size_t>(1, 40)(aGenerator);
    const auto aSpread = std::uniform_real_distribution<double>(0.1, 100.0)(aGenerator);
    std::uniform_real_distribution<double> aValue(-aSpread, aSpread);
    std::uniform_real_distribution<double> aWidth(0.0, 2.0);

    Values aValues(aSize);
    Values aWidths(aSize);
    for (std::size_t i = 0; i < aSize; ++i) {
      aValues[i] = aValue(aGenerator);
      aWidths[i] = aWidth(aGenerator);
    }

    const Classes aClasses = CompatibilityClasses::Build(aValues, aWidths);
    ASSERT_TRUE(IsSamePartition(aClasses, BruteForceComponents(aValues, aWidths))) << "iteration " << anIter;

    // Ids are dense from 1: every id from 1 to the largest one is used, and 0 never is.
    const std::size_t aClassCount = *std::max_element(aClasses.begin(), aClasses.end());
    std::vector<bool> isUsed(aClassCount + 1, false);
    for (const auto aClass : aClasses) {
      isUsed[aClass] = true;
    }
    ASSERT_FALSE(isUsed.front()) << "iteration " << anIter;
    ASSERT_TRUE(std::all_of(isUsed.begin() + 1, isUsed.end(), [](bool theFlag) { return theFlag; }))
        << "iteration " << anIter;
  }
}
