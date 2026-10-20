#include "psf/MirrorAxesWriter.hxx"

#include <array>
#include <charconv>
#include <fstream>
#include <string_view>


namespace psf {

namespace {

void WriteNumber(std::ostream& theStream, BaseTraits::float_type theValue) {
  // Adding +0.0 turns -0.0 into +0.0, so a computed midpoint never prints as "-0".
  std::array<char, 32> aBuffer{};
  const auto aResult = std::to_chars(aBuffer.data(), aBuffer.data() + aBuffer.size(), theValue + 0.0);
  theStream << std::string_view(aBuffer.data(), static_cast<std::size_t>(aResult.ptr - aBuffer.data()));
}

void WriteAxisAnchorPoint(std::ostream& theStream, const AxisAnchorPoint& thePoint) {
  if (thePoint.IsVertex()) {
    theStream << "point " << thePoint.FirstVertex();
  } else {
    theStream << "edge " << thePoint.FirstVertex() << '-' << thePoint.SecondVertex();
  }
  theStream << " X=";
  WriteNumber(theStream, thePoint.Point().X());
  theStream << " Y=";
  WriteNumber(theStream, thePoint.Point().Y());
}

void WritePlainLine(std::ostream& theStream, const MirrorAxis& theAxis) {
  WriteNumber(theStream, theAxis.First().Point().X());
  theStream << ' ';
  WriteNumber(theStream, theAxis.First().Point().Y());
  theStream << " - ";
  WriteNumber(theStream, theAxis.Second().Point().X());
  theStream << ' ';
  WriteNumber(theStream, theAxis.Second().Point().Y());
  theStream << '\n';
}

void WriteVerboseLine(std::ostream& theStream, const MirrorAxis& theAxis) {
  theStream << (theAxis.Status() == AxisStatus::Verified ? "verified" : "ambiguous") << ' ';
  WriteAxisAnchorPoint(theStream, theAxis.First());
  theStream << ' ';
  WriteAxisAnchorPoint(theStream, theAxis.Second());
  theStream << ' ';
  WriteNumber(theStream, theAxis.Deviation());
  theStream << '\n';
}

void WriteLines(std::ostream& theStream, const std::vector<MirrorAxis>& theAxes, OutputFormat theFormat) {
  if (theAxes.empty()) {
    theStream << "non-symmetric\n";
    return;
  }
  for (const auto& anAxis : theAxes) {
    if (theFormat == OutputFormat::Plain) {
      WritePlainLine(theStream, anAxis);
    } else {
      WriteVerboseLine(theStream, anAxis);
    }
  }
}

}


void MirrorAxesWriter::Write(std::ostream& theStream, const std::vector<MirrorAxis>& theAxes, OutputFormat theFormat) {
  WriteLines(theStream, theAxes, theFormat);
  if (!theStream.flush()) {
    throw MirrorAxesWriterError("write failed");
  }
}

void MirrorAxesWriter::Write(const std::filesystem::path& thePath, const std::vector<MirrorAxis>& theAxes,
                             OutputFormat theFormat) {
  std::ofstream aFile(thePath);
  if (!aFile) {
    throw MirrorAxesWriterError(thePath.string() + ": cannot open for writing");
  }
  WriteLines(aFile, theAxes, theFormat);
  aFile.close();
  if (!aFile) {
    throw MirrorAxesWriterError(thePath.string() + ": write failed");
  }
}

}
