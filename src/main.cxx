#include "psf/CommandLine.hxx"
#include "psf/MirrorAxesWriter.hxx"
#include "psf/PointsReader.hxx"
#include "psf/Polygon.hxx"
#include "psf/SymmetryFinder.hxx"

#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>
#include <vector>


namespace {

//! Exit codes, as listed in CommandLine::Help().
enum class ExitCode : int {
  Success       = 0,
  UsageError    = 1,
  InputError    = 2,
  PolygonError  = 3,
  OutputError   = 4,
  InternalError = 5
};

int ExitSucces() {
  return static_cast<int>(ExitCode::Success);
}

int Fail(ExitCode theCode, std::string_view theMessage) {
  std::cerr << "psf: " << theMessage << std::endl;
  return static_cast<int>(theCode);
}

int Fail(ExitCode theCode, std::string_view theMessage, std::string_view theSecondMessage) {
  int aResult = Fail(theCode, theMessage);
  std::cerr << theSecondMessage << std::endl;
  return aResult;
}

}


int main(int argc, char** argv) {
  // Kept outside the try block so the polygon error message can name the file.
  std::filesystem::path anInputPath;
  try {
    const psf::CommandLine aCommandLine(argc, argv);
    if (aCommandLine.IsHelp()) {
      std::cout << aCommandLine.Help();
      return ExitSucces();
    }
    anInputPath = aCommandLine.InputPath();

    // r = τ (Precision unless -t is given): cleanup may move the outline by up to τ, the distance matching allows.
    const auto aTolerance = aCommandLine.Tolerance();
    const psf::Polygon aPolygon(psf::PointsReader::Read(anInputPath), aTolerance);
    auto anAxes = psf::SymmetryFinder::Find(aPolygon, aTolerance);
    const auto aFormat = aCommandLine.IsVerbose() ? psf::OutputFormat::Verbose : psf::OutputFormat::Plain;
    if (const std::optional<std::filesystem::path>& anOutputPath = aCommandLine.OutputPath()) {
      psf::MirrorAxesWriter::Write(*anOutputPath, anAxes, aFormat);
    } else {
      psf::MirrorAxesWriter::Write(std::cout, anAxes, aFormat);
    }
    return ExitSucces();
  } catch (const psf::CommandLineError& anError) {
    return Fail(ExitCode::UsageError, anError.what(), anError.Usage());
  } catch (const psf::PointsReaderError& anError) {
    return Fail(ExitCode::InputError, anError.what());
  } catch (const psf::PolygonError& anError) {
    return Fail(ExitCode::PolygonError, anInputPath.string(), anError.what());
  } catch (const psf::MirrorAxesWriterError& anError) {
    return Fail(ExitCode::OutputError, anError.what());
  } catch (const std::exception& anError) {
    return Fail(ExitCode::InternalError, "internal error:", anError.what());
  }
}
