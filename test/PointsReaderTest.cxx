#include "psf/PointsReader.hxx"

#include <gtest/gtest.h>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>


namespace {

using psf::Point2d;
using psf::PointsReader;
using psf::PointsReaderError;
using ErrorKind = psf::PointsReaderError::ErrorKind;

//! Parses theBuffer and checks it throws an error of theKind at theLine:theColumn.
void ExpectParseError(std::string_view theBuffer, ErrorKind theKind, std::size_t theLine, std::size_t theColumn) {
  try {
    PointsReader::Parse(theBuffer);
    ADD_FAILURE() << "no error thrown for \"" << theBuffer << '"';
  } catch (const PointsReaderError& anError) {
    EXPECT_EQ(anError.Kind(), theKind) << anError.what();
    EXPECT_EQ(anError.Line(), theLine) << anError.what();
    EXPECT_EQ(anError.Column(), theColumn) << anError.what();
  }
}

//! Calls theCall, which must throw PointsReaderError, and returns the caught error.
template <typename TCall>
PointsReaderError CatchError(TCall theCall) {
  try {
    theCall();
  } catch (const PointsReaderError& anError) {
    return anError;
  }
  ADD_FAILURE() << "no error thrown";
  return PointsReaderError(ErrorKind::FileOpen, 0, 0, "<none>");
}

//! Creates a file with given content in the temp directory and removes it on destruction.
class TempFile {


public:

  TempFile(const std::string& theName, std::string_view theContent)
  : myPath(std::filesystem::temp_directory_path() / ("psf_PointsReaderTest_" + theName)) {
    std::ofstream aStream(myPath, std::ios::binary);
    aStream.write(theContent.data(), static_cast<std::streamsize>(theContent.size()));
  }

  ~TempFile() {
    std::error_code anEc;
    std::filesystem::remove(myPath, anEc);
  }

  TempFile(const TempFile&) = delete;
  TempFile& operator=(const TempFile&) = delete;

  const std::filesystem::path& Path() const noexcept {
    return myPath;
  }

private:

  std::filesystem::path myPath;

};

}


TEST(PointsReaderTest, ParsesIntegersAsDoubles) {
  EXPECT_EQ(PointsReader::Parse("1 2\n42 -7\n"), (std::vector<Point2d>{{1, 2}, {42, -7}}));
}

TEST(PointsReaderTest, ParsesFixedAndExponentForms) {
  EXPECT_EQ(PointsReader::Parse("1.5 -2.25\n.5 5.\n-1.25E+3 4e-2\n1e-310 0\n"),
            (std::vector<Point2d>{{1.5, -2.25}, {0.5, 5.0}, {-1250.0, 0.04}, {1e-310, 0}}));
}

TEST(PointsReaderTest, AcceptsTabsAndRunsOfWhitespace) {
  EXPECT_EQ(PointsReader::Parse("1\t2\n3 \t  4\n"), (std::vector<Point2d>{{1, 2}, {3, 4}}));
}

TEST(PointsReaderTest, TrimsLines) {
  EXPECT_EQ(PointsReader::Parse("   1 2   \n\t3 4\t\n"), (std::vector<Point2d>{{1, 2}, {3, 4}}));
}

TEST(PointsReaderTest, AcceptsCrLfLineEndings) {
  EXPECT_EQ(PointsReader::Parse("1 2\r\n3 4\r\n"), (std::vector<Point2d>{{1, 2}, {3, 4}}));
}

TEST(PointsReaderTest, AcceptsMissingFinalNewline) {
  EXPECT_EQ(PointsReader::Parse("1 2\n3 4"), (std::vector<Point2d>{{1, 2}, {3, 4}}));
}

TEST(PointsReaderTest, SkipsBlankLines) {
  EXPECT_EQ(PointsReader::Parse("\n\n1 2\n  \t \n\r\n3 4\n\n"), (std::vector<Point2d>{{1, 2}, {3, 4}}));
}

TEST(PointsReaderTest, SkipsComments) {
  EXPECT_EQ(PointsReader::Parse("# header\n1 2 # corner\n  # indented\n3 4#glued\n"),
            (std::vector<Point2d>{{1, 2}, {3, 4}}));
}

TEST(PointsReaderTest, EmptyInputGivesNoPoints) {
  EXPECT_TRUE(PointsReader::Parse("").empty());
  EXPECT_TRUE(PointsReader::Parse("# only comments\n\n   \n").empty());
}

TEST(PointsReaderTest, ReservesOnePointPerLine) {
  EXPECT_EQ(PointsReader::Parse("1 2\n# comment\n3 4").capacity(), 3u);
}

TEST(PointsReaderTest, RejectsLeadingPlus) {
  ExpectParseError("+1 2\n", ErrorKind::BadNumber, 1, 1);
  ExpectParseError("1 +2\n", ErrorKind::BadNumber, 1, 3);
}

TEST(PointsReaderTest, RejectsNonNumbers) {
  ExpectParseError("abc 2\n", ErrorKind::BadNumber, 1, 1);
  ExpectParseError("1 -\n", ErrorKind::BadNumber, 1, 3);
  ExpectParseError("0x10 2\n", ErrorKind::BadNumber, 1, 2);
}

TEST(PointsReaderTest, RejectsTextGluedToNumber) {
  ExpectParseError("1.5abc 2\n", ErrorKind::BadNumber, 1, 4);
  ExpectParseError("1.5,2\n", ErrorKind::BadNumber, 1, 4);
  ExpectParseError("1 2abc\n", ErrorKind::BadNumber, 1, 4);
  ExpectParseError("1e 2\n", ErrorKind::BadNumber, 1, 2);
}

TEST(PointsReaderTest, RejectsMissingCoordinate) {
  ExpectParseError("1\n", ErrorKind::MissingCoordinate, 1, 2);
  ExpectParseError("  1   # only X\n", ErrorKind::MissingCoordinate, 1, 7);
}

TEST(PointsReaderTest, RejectsExtraToken) {
  ExpectParseError("1 2 3\n", ErrorKind::ExtraToken, 1, 5);
  ExpectParseError("1 2 x\n", ErrorKind::ExtraToken, 1, 5);
}

TEST(PointsReaderTest, RejectsNonFinite) {
  ExpectParseError("nan 1\n", ErrorKind::NonFinite, 1, 1);
  ExpectParseError("1 inf\n", ErrorKind::NonFinite, 1, 3);
  ExpectParseError("-infinity 1\n", ErrorKind::NonFinite, 1, 1);
}

TEST(PointsReaderTest, RejectsOutOfRange) {
  ExpectParseError("1e400 0\n", ErrorKind::OutOfRange, 1, 1);
  ExpectParseError("0 -1e400\n", ErrorKind::OutOfRange, 1, 3);
  ExpectParseError("1e-400 0\n", ErrorKind::OutOfRange, 1, 1);
}

TEST(PointsReaderTest, ReportsLineOfFirstError) {
  ExpectParseError("1 2\n\n# c\n3 x\n4\n", ErrorKind::BadNumber, 4, 3);
  ExpectParseError("1 2\r\n3\r\n", ErrorKind::MissingCoordinate, 2, 2);
}

TEST(PointsReaderTest, CountsTabAsOneColumn) {
  ExpectParseError("\t1\tx\r\n", ErrorKind::BadNumber, 1, 4);
}

TEST(PointsReaderTest, ParseMessageHasLineAndColumn) {
  const PointsReaderError anError = CatchError([] { PointsReader::Parse("1 2\n3 x\n"); });
  EXPECT_EQ(std::string(anError.what()).rfind("2:3: ", 0), 0u) << anError.what();
}

TEST(PointsReaderTest, ReadsFile) {
  const TempFile aFile("valid.txt", "# points\n1 2\r\n3.5 -4e1\n");
  EXPECT_EQ(PointsReader::Read(aFile.Path()), (std::vector<Point2d>{{1, 2}, {3.5, -40}}));
}

TEST(PointsReaderTest, ReadsEmptyFile) {
  const TempFile aFile("empty.txt", "");
  EXPECT_TRUE(PointsReader::Read(aFile.Path()).empty());
}

TEST(PointsReaderTest, ReadMessageHasPathLineAndColumn) {
  const TempFile aFile("bad.txt", "1 2\n3 x\n");
  const PointsReaderError anError = CatchError([&] { PointsReader::Read(aFile.Path()); });
  EXPECT_EQ(anError.Kind(), ErrorKind::BadNumber);
  EXPECT_EQ(anError.Line(), 2u);
  EXPECT_EQ(anError.Column(), 3u);
  EXPECT_EQ(std::string(anError.what()).rfind(aFile.Path().string() + ":2:3: ", 0), 0u) << anError.what();
}

TEST(PointsReaderTest, MissingFileIsFileOpenError) {
  const std::filesystem::path aPath = std::filesystem::temp_directory_path() / "psf_PointsReaderTest_missing.txt";
  const PointsReaderError anError = CatchError([&] { PointsReader::Read(aPath); });
  EXPECT_EQ(anError.Kind(), ErrorKind::FileOpen);
  EXPECT_EQ(anError.Line(), 0u);
  EXPECT_EQ(anError.Column(), 0u);
  EXPECT_EQ(std::string(anError.what()).rfind(aPath.string() + ": ", 0), 0u) << anError.what();
}

TEST(PointsReaderTest, DirectoryIsFileOpenError) {
  const PointsReaderError anError = CatchError([] { PointsReader::Read(std::filesystem::temp_directory_path()); });
  EXPECT_EQ(anError.Kind(), ErrorKind::FileOpen);
}
