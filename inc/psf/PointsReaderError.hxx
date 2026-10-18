#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>


namespace psf {

//! Error raised by PointsReader; carries the failure kind and its 1-based position in the input.
class PointsReaderError : public std::runtime_error {


public:

  enum class ErrorKind {
    FileOpen,           //!< path is missing, not a regular file, or cannot be opened
    ReadFailed,         //!< size query or read of the file failed
    BadNumber,          //!< token is not a number, has a leading '+', or has text glued to it
    MissingCoordinate,  //!< line holds X but no Y
    ExtraToken,         //!< line holds more than two values
    NonFinite,          //!< value is nan or inf
    OutOfRange          //!< value overflows or underflows double
  };

  //! theWhat is the complete message returned by what().
  PointsReaderError(ErrorKind theKind, std::size_t theLine, std::size_t theColumn, const std::string& theWhat)
  : std::runtime_error(theWhat), myKind(theKind), myLine(theLine), myColumn(theColumn) {}

  ErrorKind Kind() const noexcept {
    return myKind;
  }

  //! 1-based line number; 0 for file-level errors.
  std::size_t Line() const noexcept {
    return myLine;
  }

  //! 1-based byte offset in the untrimmed line; 0 for file-level errors.
  std::size_t Column() const noexcept {
    return myColumn;
  }

private:

  ErrorKind   myKind;
  std::size_t myLine;
  std::size_t myColumn;

};


}
