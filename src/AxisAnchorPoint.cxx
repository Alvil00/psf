#include "psf/AxisAnchorPoint.hxx"


namespace psf {

AxisAnchorPoint::AxisAnchorPoint(const Polygon& thePolygon, std::size_t theElement) {
  const auto& aVertices   = thePolygon.Vertices();
  const auto& anOriginals = thePolygon.OriginalIndices();
  const auto  aSize       = thePolygon.Size();
  const auto  anElement   = theElement % (2 * aSize);
  const auto  aFirst      = anElement / 2;
  if (anElement % 2 == 0) {
    myPoint        = aVertices[aFirst];
    myFirstVertex  = anOriginals[aFirst];
    mySecondVertex = anOriginals[aFirst];
    return;
  }
  const std::size_t aSecond = (aFirst + 1) % aSize;
  myPoint        = aVertices[aFirst] + 0.5 * thePolygon.Edges()[aFirst];
  myFirstVertex  = anOriginals[aFirst];
  mySecondVertex = anOriginals[aSecond];
}


}
