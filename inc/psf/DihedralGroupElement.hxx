#pragma once

#include "psf/BaseTraits.hxx"
#include "psf/Matrix2d.hxx"
#include "psf/Point2d.hxx"
#include "psf/Polygon.hxx"

#include <cstddef>


namespace psf {

//! An element of the estimated dihedral group D_K of a polygon: the isometry q = c + M (p - c) about the centre c,
//! together with the vertex-index map it stands for: i -> (shift + i) mod N for a rotation, i -> (shift - i) mod N
//! for a reflection. The rotations form the cyclic subgroup C_K; the reflections are its other coset.
class DihedralGroupElement {


public:

  using value_type = BaseTraits::float_type;

  //! The neutral element: identity matrix, shift 0, centre at the origin.
  DihedralGroupElement() noexcept = default;

  //! Takes the parts as given; nothing is checked. theIsCyclicSubgroupElement is true for a rotation,
  //! false for a reflection.
  DihedralGroupElement(const Matrix2d& theMatrix, std::size_t theShift, bool theIsCyclicSubgroupElement,
                       const Point2d& theCenter) noexcept
  : myMatrix(theMatrix),
    myShift(theShift),
    myIsCyclicSubgroupElement(theIsCyclicSubgroupElement),
    myCenter(theCenter) {}

  //! Vertex that the mirror centred on symbol element theCenter (in [0, N)) pairs with vertex theIndex.
  //! Element 2i is vertex i, so element 2i maps to element 2 * theCenter - 2i, which is vertex theCenter - i.
  static std::size_t MirrorPartner(std::size_t theCenter, std::size_t theIndex, std::size_t theSize) noexcept {
    return (theCenter + theSize - theIndex) % theSize;
  }

  const Matrix2d& Matrix() const noexcept {
    return myMatrix;
  }

  std::size_t Shift() const noexcept {
    return myShift;
  }

  //! True for a rotation, false for a reflection.
  bool IsCyclicSubgroupElement() const noexcept {
    return myIsCyclicSubgroupElement;
  }

  const Point2d& Center() const noexcept {
    return myCenter;
  }

  //! The image of thePoint under the isometry: c + M (thePoint - c).
  Point2d Apply(const Point2d& thePoint) const noexcept {
    return myCenter + myMatrix * (thePoint - myCenter);
  }

  //! The image of vertex index theIndex under the index map, for a polygon of theSize vertices.
  std::size_t MapIndex(std::size_t theIndex, std::size_t theSize) const noexcept {
    return myIsCyclicSubgroupElement ? (theIndex + myShift) % theSize : MirrorPartner(myShift, theIndex, theSize);
  }

  //! |v_g(r) - g v_r| for r = theRoot: how far the image of vertex theRoot lands from the vertex its index map
  //! sends it to.
  value_type Deviation(const Polygon& thePolygon, std::size_t theRoot) const {
    std::size_t anImage = 0;
    return Deviation(thePolygon, theRoot, anImage);
  }

  //! The same deviation; also writes MapIndex(theRoot, N) to theImage, so the caller need not compute it again.
  value_type Deviation(const Polygon& thePolygon, std::size_t theRoot, std::size_t& theImage) const;

private:

  Matrix2d    myMatrix                  = Matrix2d::Identity();
  std::size_t myShift                   = 0;
  bool        myIsCyclicSubgroupElement = true;
  Point2d     myCenter;
};


}
