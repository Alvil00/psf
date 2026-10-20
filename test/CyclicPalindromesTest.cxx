#include "psf/CyclicPalindromes.hxx"

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <vector>


namespace {

using psf::CyclicPalindromes;
using Centers = std::vector<std::size_t>;

//! Test oracle: checks every centre and every pair directly in O(M^2).
Centers BruteForceCenters(const std::vector<int>& theSequence) {
  const auto aSize = theSequence.size();
  Centers aCenters;
  for (std::size_t c = 0; c < aSize / 2; ++c) {
    bool isMirror = true;
    for (std::size_t k = 1; k < aSize && isMirror; ++k) {
      isMirror = theSequence[(c + k) % aSize] == theSequence[(c + aSize - k) % aSize];
    }
    if (isMirror) {
      aCenters.push_back(c);
    }
  }
  return aCenters;
}

}


TEST(CyclicPalindromesTest, RejectsEmptyAndOddSizes) {
  EXPECT_THROW(CyclicPalindromes::Centers(std::vector<int>{}), std::invalid_argument);
  EXPECT_THROW(CyclicPalindromes::Centers(std::vector<int>{1}), std::invalid_argument);
  EXPECT_THROW(CyclicPalindromes::Centers(std::vector<int>{1, 2, 3}), std::invalid_argument);
}

TEST(CyclicPalindromesTest, AllEqualHasEveryMirror) {
  EXPECT_EQ(CyclicPalindromes::Centers(std::vector<int>(6, 7)), (Centers{0, 1, 2}));
}

TEST(CyclicPalindromesTest, TwoElementsHaveOneMirror) {
  // Element 0 maps to itself and element 1 maps to itself: any pair is mirrored about either element.
  EXPECT_EQ(CyclicPalindromes::Centers(std::vector<int>{1, 2}), (Centers{0}));
}

TEST(CyclicPalindromesTest, PeriodicSequenceHasMirrorsHalfAPeriodApart) {
  // Period 4, so mirrors repeat every 2 elements.
  EXPECT_EQ(CyclicPalindromes::Centers(std::vector<int>{0, 1, 2, 1, 0, 1, 2, 1}), (Centers{0, 2}));
}

TEST(CyclicPalindromesTest, AsymmetricSequenceHasNoMirror) {
  EXPECT_TRUE(CyclicPalindromes::Centers(std::vector<int>{0, 1, 2, 3}).empty());
}

TEST(CyclicPalindromesTest, LinearPalindromeBetweenElementsIsNotReported) {
  // 1 2 2 1 reads the same backwards, but its centre lies between two elements.
  EXPECT_TRUE(CyclicPalindromes::Centers(std::vector<int>{1, 2, 2, 1}).empty());
}

TEST(CyclicPalindromesTest, MirrorWrapsAroundTheSequenceEnd) {
  // Mirror about element 0: pairs (1, 5) and (2, 4) cross the end of the vector.
  EXPECT_EQ(CyclicPalindromes::Centers(std::vector<int>{5, 1, 2, 9, 2, 1}), (Centers{0}));
}

TEST(CyclicPalindromesTest, MirrorAtLastReportedCentre) {
  EXPECT_EQ(CyclicPalindromes::Centers(std::vector<int>{4, 5, 8, 5, 4, 6}), (Centers{2}));
}

TEST(CyclicPalindromesTest, UsesTheGivenComparator) {
  // Equal modulo 10 is an equivalence: under it the sequence reads 5 1 2 9 2 1, mirrored about element 0.
  const std::vector<int> aSequence{5, 1, 2, 9, 12, 11};
  const auto anEqualModTen = [](int theLeft, int theRight) { return theLeft % 10 == theRight % 10; };
  EXPECT_EQ(CyclicPalindromes::Centers(aSequence, anEqualModTen), (Centers{0}));
  EXPECT_TRUE(CyclicPalindromes::Centers(aSequence).empty());
}

TEST(CyclicPalindromesTest, ToleranceComparatorReportsAMirrorThatIsNotThere) {
  // |a - b| <= 1 is not transitive: 0 ~ 1 ~ 2, but 0 and 2 are not near. About element 2 the pair (1, 3) is 0 and 2,
  // so element 2 is no mirror, yet Manacher copies its radius from the mirror image of a palindrome built on that
  // chain and never compares the pair. Copied radii only overshoot, so mirrors are added, never lost.
  const std::vector<double> aSequence{3, 0, 3, 2, 3, 1};
  const auto aNear = [](double theLeft, double theRight) { return std::abs(theLeft - theRight) <= 1.0; };
  ASSERT_FALSE(aNear(aSequence[1], aSequence[3]));
  EXPECT_EQ(CyclicPalindromes::Centers(aSequence, aNear), (Centers{0, 1, 2}));
}

TEST(CyclicPalindromesTest, RegularHexagonSymbolStringHasSixMirrors) {
  // Interleaved vertex angle / edge length symbols: 120 deg corners, unit edges.
  const std::vector<int> aSymbols{120, 1, 120, 1, 120, 1, 120, 1, 120, 1, 120, 1};
  EXPECT_EQ(CyclicPalindromes::Centers(aSymbols), (Centers{0, 1, 2, 3, 4, 5}));
}

TEST(CyclicPalindromesTest, RectangleSymbolStringHasTwoEdgeMirrors) {
  // 2 x 1 rectangle: both axes pass through edge midpoints (odd centres), none through corners.
  const std::vector<int> aSymbols{90, 2, 90, 1, 90, 2, 90, 1};
  EXPECT_EQ(CyclicPalindromes::Centers(aSymbols), (Centers{1, 3}));
}

TEST(CyclicPalindromesTest, MatchesBruteForceOnRandomSequences) {
  std::mt19937 aGenerator(20261018);
  for (int anIter = 0; anIter < 5000; ++anIter) {
    const auto aSize      = 2 * std::uniform_int_distribution<std::size_t>(1, 12)(aGenerator);
    const auto anAlphabet = std::uniform_int_distribution<int>(1, 3)(aGenerator);
    std::uniform_int_distribution<int> aSymbol(0, anAlphabet - 1);

    std::vector<int> aSequence(aSize);
    for (auto& aValue : aSequence) {
      aValue = aSymbol(aGenerator);
    }
    ASSERT_EQ(CyclicPalindromes::Centers(aSequence), BruteForceCenters(aSequence)) << "size " << aSize;
  }
}
