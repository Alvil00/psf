#include "psf/PointsReader.hxx"

#include "psf/DebugOutput.hxx"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iterator>
#include <string>
#include <system_error>


namespace psf {

namespace {

using ErrorKind = PointsReaderError::ErrorKind;
using Iterator  = std::string_view::const_iterator;

//! One line of the buffer; myBegin is the untrimmed line start that columns are counted from.
struct LineView {
  Iterator    myBegin;
  Iterator    myEnd;
  std::size_t myNumber;
};

[[noreturn]] void ThrowAt(ErrorKind theKind, const LineView& theLine, Iterator thePos, std::string_view theMessage) {
  const auto aColumn = static_cast<std::size_t>(thePos - theLine.myBegin) + 1;
  std::string aWhat = std::to_string(theLine.myNumber) + ':' + std::to_string(aColumn) + ": ";
  aWhat += theMessage;
  throw PointsReaderError(theKind, theLine.myNumber, aColumn, aWhat);
}

[[noreturn]] void ThrowFile(ErrorKind theKind, const std::filesystem::path& thePath, std::string_view theMessage) {
  std::string aWhat = thePath.string() + ": ";
  aWhat += theMessage;
  throw PointsReaderError(theKind, 0, 0, aWhat);
}

bool IsBlank(char theChar) noexcept {
  return theChar == ' ' || theChar == '\t';
}

Iterator SkipBlanks(Iterator thePos, Iterator theEnd) {
  return std::find_if_not(thePos, theEnd, IsBlank);
}

//! Parses one coordinate at thePos (not the line end); the value must end at the line end or at a blank.
Iterator ParseValue(const LineView& theLine, Iterator thePos, Point2d::value_type& theValue) {
  if (*thePos == '+') {
    ThrowAt(ErrorKind::BadNumber, theLine, thePos, "leading '+' is not allowed");
  }

  // std::from_chars works on pointers only, so the iterator range is converted here.
  // thePos points at a character and may be dereferenced. theLine.myEnd may be the buffer's end iterator,
  // and *end() is undefined behaviour (MSVC debug iterators assert on it), so aLast is computed by
  // offsetting aFirst instead.
  const char* const aFirst = &(*thePos);
  const char* const aLast  = aFirst + (theLine.myEnd - thePos);

  const auto aResult = std::from_chars(aFirst, aLast, theValue, std::chars_format::general);
  if (aResult.ec == std::errc::invalid_argument) {
    ThrowAt(ErrorKind::BadNumber, theLine, thePos, "expected a number");
  }
  if (aResult.ec == std::errc::result_out_of_range) {
    ThrowAt(ErrorKind::OutOfRange, theLine, thePos, "number is out of double range");
  }
  if (!std::isfinite(theValue)) {
    ThrowAt(ErrorKind::NonFinite, theLine, thePos, "nan and inf are not allowed");
  }

  const auto aValueEnd = thePos + (aResult.ptr - aFirst);
  if (aValueEnd != theLine.myEnd && !IsBlank(*aValueEnd)) {
    ThrowAt(ErrorKind::BadNumber, theLine, aValueEnd, "unexpected character after number");
  }
  return aValueEnd;
}

//! Appends the point held by theLine (already cut at '#'); blank lines add nothing.
void ParseLine(const LineView& theLine, std::vector<Point2d>& thePoints) {
  auto aPos = SkipBlanks(theLine.myBegin, theLine.myEnd);
  if (aPos == theLine.myEnd) {
    return;
  }

  Point2d::value_type aX = 0.0;
  aPos = SkipBlanks(ParseValue(theLine, aPos, aX), theLine.myEnd);
  if (aPos == theLine.myEnd) {
    ThrowAt(ErrorKind::MissingCoordinate, theLine, aPos, "expected Y coordinate");
  }

  Point2d::value_type aY = 0.0;
  aPos = SkipBlanks(ParseValue(theLine, aPos, aY), theLine.myEnd);
  if (aPos != theLine.myEnd) {
    ThrowAt(ErrorKind::ExtraToken, theLine, aPos, "expected end of line after Y coordinate");
  }

  thePoints.emplace_back(aX, aY);
}

}


std::vector<Point2d> PointsReader::Read(const std::filesystem::path& thePath) {
  std::error_code anEc;
  if (!std::filesystem::is_regular_file(thePath, anEc)) {
    ThrowFile(ErrorKind::FileOpen, thePath, anEc ? anEc.message() : "not an existing regular file");
  }

  const auto aSize = std::filesystem::file_size(thePath, anEc);
  if (anEc) {
    ThrowFile(ErrorKind::ReadFailed, thePath, anEc.message());
  }

  // Binary mode reads the bytes exactly as stored, so the read count equals aSize and the short-read
  // check below stays meaningful. Text mode on Windows turns "\r\n" into "\n" and would deliver fewer
  // bytes for every CRLF file. Parse strips '\r' itself.
  std::ifstream aStream(thePath, std::ios::binary);
  if (!aStream) {
    ThrowFile(ErrorKind::FileOpen, thePath, "cannot open file");
  }

  std::string aBuffer(aSize, '\0');
  aStream.read(aBuffer.data(), aBuffer.size());
  if (static_cast<std::size_t>(aStream.gcount()) != aSize) {
    ThrowFile(ErrorKind::ReadFailed, thePath, "file is shorter than its reported size");
  }

  try {
    return Parse(aBuffer);
  } catch (const PointsReaderError& anError) {
    throw PointsReaderError(anError.Kind(), anError.Line(), anError.Column(),
                            thePath.string() + ':' + anError.what());
  }
}

std::vector<Point2d> PointsReader::Parse(std::string_view theBuffer) {

  if (theBuffer.empty()) {
    return {};
  }

  auto       aPos  = theBuffer.begin();
  const auto anEnd = theBuffer.end();

  auto aLineCount = static_cast<std::size_t>(std::count(aPos, anEnd, '\n'));
  if (theBuffer.back() != '\n') {
    ++aLineCount;
  }

  std::vector<Point2d> aPoints;
  aPoints.reserve(aLineCount);

  for (std::size_t aLineNumber = 1; aPos != anEnd; ++aLineNumber) {
    auto       aLineEnd = std::find(aPos, anEnd, '\n');
    const auto aNext    = aLineEnd == anEnd ? anEnd : aLineEnd + 1;
    if (aLineEnd != aPos && *std::prev(aLineEnd) == '\r') {
      --aLineEnd;
    }

    ParseLine(LineView{aPos, std::find(aPos, aLineEnd, '#'), aLineNumber}, aPoints);
    aPos = aNext;
  }

#if PSF_DEBUG_OUTPUT_ENABLED
  DebugOutput::Line("read") << aPoints.size() << " points\n";
  for (std::size_t i = 0; i < aPoints.size(); ++i) {
    DebugOutput::Line("read") << "input " << i << ' ' << DebugOutput::Point(aPoints[i]) << '\n';
  }
#endif
  return aPoints;
}

}
