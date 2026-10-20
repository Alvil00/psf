#include "psf/MirrorAxesWriter.hxx"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>


namespace {

using psf::AxisStatus;
using psf::AxisAnchorPoint;
using psf::MirrorAxis;
using psf::OutputFormat;
using psf::Point2d;

std::string Written(const std::vector<MirrorAxis>& theAxes, OutputFormat theFormat) {
  std::ostringstream aStream;
  psf::MirrorAxesWriter::Write(aStream, theAxes, theFormat);
  return aStream.str();
}

}


TEST(MirrorAxesWriterTest, NoAxesWritesNonSymmetricInBothFormats) {
  EXPECT_EQ(Written({}, OutputFormat::Plain), "non-symmetric\n");
  EXPECT_EQ(Written({}, OutputFormat::Verbose), "non-symmetric\n");
}

TEST(MirrorAxesWriterTest, PlainWritesTheTwoAnchorPointsOnePerLine) {
  const std::vector<MirrorAxis> anAxes{
    MirrorAxis{AxisAnchorPoint{Point2d(0, 1), 0, 0}, AxisAnchorPoint{Point2d(1, 0), 2, 2}, AxisStatus::Verified, 0.0},
    MirrorAxis{AxisAnchorPoint{Point2d(0.5, 0), 0, 1}, AxisAnchorPoint{Point2d(-1, 3), 2, 2}, AxisStatus::Ambiguous,
               0.0066},
  };
  EXPECT_EQ(Written(anAxes, OutputFormat::Plain),
            "0 1 - 1 0\n"
            "0.5 0 - -1 3\n");
}

TEST(MirrorAxesWriterTest, PlainNumbersRoundTripAndNegativeZeroPrintsAsZero) {
  const std::vector<MirrorAxis> anAxes{
    MirrorAxis{AxisAnchorPoint{Point2d(-0.0, 1.0 / 3), 0, 0}, AxisAnchorPoint{Point2d(1e-300, -2.5), 1, 1},
               AxisStatus::Verified, 1e-17},
  };
  EXPECT_EQ(Written(anAxes, OutputFormat::Plain), "0 0.3333333333333333 - 1e-300 -2.5\n");
}

TEST(MirrorAxesWriterTest, VerboseWritesPointsAndEdgesOnePerLine) {
  const std::vector<MirrorAxis> anAxes{
    MirrorAxis{AxisAnchorPoint{Point2d(0, 0), 0, 0}, AxisAnchorPoint{Point2d(1, 1), 2, 2}, AxisStatus::Verified, 0.0},
    MirrorAxis{AxisAnchorPoint{Point2d(0.5, 0), 0, 1}, AxisAnchorPoint{Point2d(0.01, 3), 2, 2}, AxisStatus::Ambiguous,
               0.0066},
  };
  EXPECT_EQ(Written(anAxes, OutputFormat::Verbose),
            "verified point 0 X=0 Y=0 point 2 X=1 Y=1 0\n"
            "ambiguous edge 0-1 X=0.5 Y=0 point 2 X=0.01 Y=3 0.0066\n");
}

TEST(MirrorAxesWriterTest, VerboseNumbersRoundTripAndNegativeZeroPrintsAsZero) {
  const std::vector<MirrorAxis> anAxes{
    MirrorAxis{AxisAnchorPoint{Point2d(-0.0, 1.0 / 3), 0, 0}, AxisAnchorPoint{Point2d(1e-300, -2.5), 1, 1},
               AxisStatus::Verified, 1e-17},
  };
  EXPECT_EQ(Written(anAxes, OutputFormat::Verbose),
            "verified point 0 X=0 Y=0.3333333333333333 point 1 X=1e-300 Y=-2.5 1e-17\n");
}

TEST(MirrorAxesWriterTest, BadStreamThrows) {
  std::ostringstream aStream;
  aStream.setstate(std::ios::badbit);
  try {
    psf::MirrorAxesWriter::Write(aStream, {}, OutputFormat::Plain);
    FAIL() << "expected MirrorAxesWriterError";
  } catch (const psf::MirrorAxesWriterError& anError) {
    EXPECT_STREQ(anError.what(), "write failed");
  }
}

TEST(MirrorAxesWriterTest, FileGetsTheSameTextAsTheStream) {
  const std::vector<MirrorAxis> anAxes{
    MirrorAxis{AxisAnchorPoint{Point2d(0, 0), 0, 0}, AxisAnchorPoint{Point2d(1, 1), 2, 2}, AxisStatus::Verified, 0.0},
  };
  const std::filesystem::path aPath = std::filesystem::temp_directory_path() / "psf_MirrorAxesWriterTest.txt";
  for (const auto aFormat : {OutputFormat::Plain, OutputFormat::Verbose}) {
    psf::MirrorAxesWriter::Write(aPath, anAxes, aFormat);

    std::ifstream aFile(aPath);
    const std::string aContents((std::istreambuf_iterator<char>(aFile)), std::istreambuf_iterator<char>());
    aFile.close();
    std::filesystem::remove(aPath);
    EXPECT_EQ(aContents, Written(anAxes, aFormat));
  }
}

TEST(MirrorAxesWriterTest, UnopenableFileThrows) {
  const std::filesystem::path aPath =
    std::filesystem::temp_directory_path() / "psf_MirrorAxesWriterTest_missing_dir" / "axes.txt";
  try {
    psf::MirrorAxesWriter::Write(aPath, {}, OutputFormat::Plain);
    FAIL() << "expected MirrorAxesWriterError";
  } catch (const psf::MirrorAxesWriterError& anError) {
    EXPECT_EQ(std::string(anError.what()), aPath.string() + ": cannot open for writing");
  }
}
