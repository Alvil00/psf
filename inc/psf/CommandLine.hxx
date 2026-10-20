#pragma once

#include "psf/BaseTraits.hxx"

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>


namespace psf {

//! Usage error in the command line: unknown option, missing or malformed value, wrong number of input files.
//! Carries the usage line naming the program, since the CommandLine that knows the name was never built.
class CommandLineError : public std::runtime_error {


public:

  explicit CommandLineError(const std::string& theWhat, std::string theUsage = {})
  : std::runtime_error(theWhat), myUsage(std::move(theUsage)) {}

  //! The usage line of CommandLine::Usage(); empty for an error not yet passed through the constructor.
  const std::string& Usage() const noexcept {
    return myUsage;
  }

private:

  std::string myUsage;

};

//! Options of the psf executable:
//!   psf <file> [-t|--tolerance <τ>] [-o|--output <file>] [-v|--verbose] [-h|--help]
//! With -h or --help anywhere, everything else is ignored and IsHelp() is true.
class CommandLine {


public:


  //! Parses the command line as main() receives it: theArgv[0] is the program name, the options follow.
  //! The words are read in place; nothing keeps pointers into theArgv after the constructor returns.
  //! Throws CommandLineError, with Usage() attached, on an unknown option, a repeated option, an option
  //! without its value, a tolerance that is not a finite non-negative number, or not exactly one input file.
  CommandLine(int theArgc, char** theArgv);

  bool IsHelp() const noexcept {
    return myIsHelp;
  }

  const std::filesystem::path& InputPath() const noexcept {
    return myInputPath;
  }

  //! The file to write the axes to; empty for standard output.
  const std::optional<std::filesystem::path>& OutputPath() const noexcept {
    return myOutputPath;
  }

  //! True with -v or --verbose: write the axes in the verbose format instead of the plain one.
  bool IsVerbose() const noexcept {
    return myIsVerbose;
  }

  //! τ, BaseTraits::Precision unless given.
  BaseTraits::float_type Tolerance() const noexcept {
    return myTolerance;
  }

  //! The name the program was started under: the file name of theArgv[0], without its directory;
  //! "psf" when theArgv[0] is missing or empty.
  const std::string& ProgramName() const noexcept {
    return myProgramName;
  }

  //! One-line usage, printed after a usage error.
  std::string Usage() const;

  //! Full help text for --help.
  std::string Help() const;

private:

  //! Reads the options after theArgv[0]; throws CommandLineError without the usage line.
  void Parse(int theArgc, char** theArgv);

  bool                                 myIsHelp    = false;
  bool                                 myIsVerbose = false;
  std::filesystem::path                myInputPath;
  std::optional<std::filesystem::path> myOutputPath;
  BaseTraits::float_type               myTolerance = BaseTraits::Precision;
  std::string                          myProgramName;

};


}
