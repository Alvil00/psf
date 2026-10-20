#include "psf/DihedralGroupElement.hxx"


namespace psf {

DihedralGroupElement::value_type DihedralGroupElement::Deviation(const Polygon& thePolygon, std::size_t theRoot,
                                                                 std::size_t& theImage) const {
  const auto& aVertices = thePolygon.Vertices();
  theImage = MapIndex(theRoot, thePolygon.Size());
  return (aVertices[theImage] - Apply(aVertices[theRoot])).Magnitude();
}


}
