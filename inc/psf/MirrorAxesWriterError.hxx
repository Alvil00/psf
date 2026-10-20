#pragma once

#include <stdexcept>
#include <string>


namespace psf {

//! Error raised by MirrorAxesWriter when the axes cannot be written.
class MirrorAxesWriterError : public std::runtime_error {


public:

  //! theWhat is the complete message returned by what().
  explicit MirrorAxesWriterError(const std::string& theWhat)
  : std::runtime_error(theWhat) {}

};


}
