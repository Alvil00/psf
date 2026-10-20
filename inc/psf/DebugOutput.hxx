#pragma once

//! Debug output of the pipeline stages is on only when DEBUG_OUTPUT is defined (CMake option PSF_DEBUG_OUTPUT)
//! and NDEBUG is not. Call sites guard their output with #if PSF_DEBUG_OUTPUT_ENABLED, so none of it is compiled
//! otherwise.
#if defined(DEBUG_OUTPUT) && !defined(NDEBUG)
#define PSF_DEBUG_OUTPUT_ENABLED 1
#else
#define PSF_DEBUG_OUTPUT_ENABLED 0
#endif

#if PSF_DEBUG_OUTPUT_ENABLED

#include "psf/BaseTraits.hxx"
#include "psf/Point2d.hxx"

#include <array>
#include <charconv>
#include <iostream>
#include <string>
#include <string_view>


namespace psf {

//! Writes debug lines to std::cout as "psf debug: <stage>: <text>".
class DebugOutput {


public:

  DebugOutput() = delete;

  //! Starts a debug line for theStage; the caller writes the text and ends the line with '\n'.
  static std::ostream& Line(std::string_view theStage) {
    return std::cout << "psf debug: " << theStage << ": ";
  }

  //! theValue in the shortest form that reads back exactly.
  static std::string Number(BaseTraits::float_type theValue) {
    std::array<char, 32> aBuffer{};
    const auto aResult = std::to_chars(aBuffer.data(), aBuffer.data() + aBuffer.size(), theValue);
    return std::string(aBuffer.data(), aResult.ptr);
  }

  //! thePoint as "(x, y)", each coordinate as in Number().
  static std::string Point(const Point2d& thePoint) {
    return '(' + Number(thePoint.X()) + ", " + Number(thePoint.Y()) + ')';
  }

};


}

#endif
