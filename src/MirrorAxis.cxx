#include "psf/MirrorAxis.hxx"


namespace psf {

MirrorAxis::MirrorAxis(const Polygon& thePolygon, std::size_t theCenter, AxisStatus theStatus,
                       value_type theDeviation)
: myFirst(thePolygon, theCenter),
  mySecond(thePolygon, theCenter + thePolygon.Size()),
  myStatus(theStatus),
  myDeviation(theDeviation) {}


}
