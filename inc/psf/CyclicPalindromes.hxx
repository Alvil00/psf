#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <vector>


namespace psf {

//! Finds the mirrors of a cyclic sequence with Manacher's algorithm in O(M).
//! A mirror about element c maps element c + k to element c - k (indices modulo M) for every k.
//! Only mirrors centred on an element are searched: for the polygon's symbol string every axis passes
//! through a vertex symbol or an edge symbol, never between two symbols.
class CyclicPalindromes {


public:

  CyclicPalindromes() = delete;

  //! Returns the centres c in [0, M/2), ascending, about which theSequence is a cyclic palindrome.
  //! For even M the centres c and c + M/2 describe the same mirror, so each mirror is reported once.
  //! theComparator(a, b) decides whether two elements match; its result must convert to bool.
  //! The default needs only TElement's operator==.
  //! theComparator must be an equivalence relation (reflexive, symmetric, transitive): Manacher's algorithm copies
  //! radii from mirrored positions instead of re-comparing their pairs, which is valid only then.
  //! A tolerance comparison such as |a - b| <= tol is not transitive and silently yields wrong centres.
  //! Throws std::invalid_argument if M is zero or odd.
  template <typename TElement, typename TCompare = std::equal_to<TElement>>
  static std::vector<std::size_t> Centers(const std::vector<TElement>& theSequence,
                                          const TCompare&              theComparator = TCompare()) {
    const auto aSize = theSequence.size();
    // In our case the second condition never be true because sum of number of vertices and number of edges always be even number
    if (aSize == 0 || aSize % 2 != 0) {
      throw std::invalid_argument("CyclicPalindromes: sequence size must be even and non-zero");
    }
    const std::size_t aHalf = aSize / 2;

    // Linear Manacher runs over the padded sequence [second half | whole sequence | first half] of length 2M,
    // so every centre c in [0, M/2) sits at padded index c + M/2 with M/2 elements of context on each side.
    // The padding is virtual: padded index j reads element (j + M/2) mod M, so no element is copied.
    const std::size_t aPaddedSize = 2 * aSize;
    const auto anAt = [&](std::size_t theIndex) -> const TElement& {
      return theSequence[(theIndex + aHalf) % aSize];
    };

    // aRadius[j] counts the elements on one side of j, centre included, of the longest palindrome around j.
    // Centres past M - 1 are never read below, so the scan stops there; earlier centres are needed for reuse.
    std::vector<std::size_t> aRadius(aSize, 0);
    std::size_t aLeft  = 0;
    std::size_t aRight = 0;  // exclusive right end of the rightmost palindrome found so far
    for (std::size_t j = 0; j < aSize; ++j) {
      std::size_t aCurrentRadius = 1;
      if (j < aRight) {
        aCurrentRadius = std::min(aRadius[aLeft + aRight - 1 - j], aRight - j);
      }
      while (aCurrentRadius <= j && j + aCurrentRadius < aPaddedSize
             && theComparator(anAt(j - aCurrentRadius), anAt(j + aCurrentRadius))) {
        ++aCurrentRadius;
      }
      aRadius[j] = aCurrentRadius;
      if (j + aCurrentRadius > aRight) {
        aLeft  = j + 1 - aCurrentRadius;
        aRight = j + aCurrentRadius;
      }
    }

    // A cyclic palindrome about c needs pairs (c - k, c + k) equal for k = 1 .. M/2 - 1;
    // k = M/2 pairs element c + M/2 with itself, and larger k repeat smaller ones.
    std::vector<std::size_t> aCenters;
    for (std::size_t c = 0; c < aHalf; ++c) {
      if (aRadius[c + aHalf] >= aHalf) {
        aCenters.push_back(c);
      }
    }
    return aCenters;
  }

};


}
