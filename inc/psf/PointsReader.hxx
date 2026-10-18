#pragma once

#include "psf/Point2d.hxx"
#include "psf/PointsReaderError.hxx"

#include <filesystem>
#include <string_view>
#include <vector>


namespace psf {

//! Reads 2D points stored one per line as "X Y".
//! Lines are cut at '#' and trimmed; blank lines are skipped. X and Y are separated by at least
//! one space or tab and use any std::from_chars general form (integer, fixed or exponent).
//! A leading '+', nan/inf, out-of-range values and extra tokens are errors; the first one throws.
class PointsReader {


public:

  PointsReader() = delete;

  //! Reads the whole file into one buffer and parses it.
  //! Throws PointsReaderError; its what() is prefixed with the path.
  static std::vector<Point2d> Read(const std::filesystem::path& thePath);

  //! Parses an in-memory buffer. Throws PointsReaderError with what() as "<line>:<col>: <message>".
  static std::vector<Point2d> Parse(std::string_view theBuffer);

};


}
