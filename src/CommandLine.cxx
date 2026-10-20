#include "psf/CommandLine.hxx"

#include <algorithm>
#include <charconv>
#include <string>
#include <cmath>
#include <system_error>


namespace psf {

namespace {

bool IsHelpFlag(std::string_view theArgument) noexcept {
  return theArgument == "-h" || theArgument == "--help";
}

BaseTraits::float_type ParseTolerance(std::string_view theText) {
  BaseTraits::float_type aValue = 0.0;
  const char* const aLast = theText.data() + theText.size();
  const auto aResult = std::from_chars(theText.data(), aLast, aValue, std::chars_format::general);
  if (theText.empty() || aResult.ec != std::errc() || aResult.ptr != aLast) {
    throw CommandLineError("tolerance '" + std::string(theText) + "' is not a number");
  }
  if (!std::isfinite(aValue) || aValue < 0.0) {
    throw CommandLineError("tolerance must be a finite number not below 0, got '" + std::string(theText) + "'");
  }
  return aValue;
}

std::string ProgramNameOf(int theArgc, char** theArgv) {
  if (theArgc < 1 || theArgv == nullptr || theArgv[0] == nullptr) {
    return "psf";
  }
  std::string aName = std::filesystem::path(theArgv[0]).filename().string();
  return aName.empty() ? "psf" : aName;
}

}


CommandLine::CommandLine(int theArgc, char** theArgv)
: myProgramName(ProgramNameOf(theArgc, theArgv)) {
  try {
    Parse(theArgc, theArgv);
  } catch (const CommandLineError& anError) {
    throw CommandLineError(anError.what(), Usage());
  }
}

void CommandLine::Parse(int theArgc, char** theArgv) {
  // theArgv[0] is the program name; the words after it are read in place, without copying.
  const int aCount = std::max(theArgc, 1);
  for (int i = 1; i < aCount; ++i) {
    if (IsHelpFlag(theArgv[i])) {
      myIsHelp = true;
      return;
    }
  }

  bool hasInput     = false;
  bool hasTolerance = false;
  for (int i = 1; i < aCount; ++i) {
    const std::string_view anArgument = theArgv[i];

    const auto aValue = [&]() -> std::string_view {
      if (i + 1 == aCount) {
        throw CommandLineError("option " + std::string(anArgument) + " needs a value");
      }
      return theArgv[++i];
    };

    if (anArgument == "-t" || anArgument == "--tolerance") {
      if (hasTolerance) {
        throw CommandLineError("tolerance is given more than once");
      }
      myTolerance  = ParseTolerance(aValue());
      hasTolerance = true;
    } else if (anArgument == "-o" || anArgument == "--output") {
      if (myOutputPath) {
        throw CommandLineError("output file is given more than once");
      }
      myOutputPath = std::filesystem::path(aValue());
    } else if (anArgument == "-v" || anArgument == "--verbose") {
      if (myIsVerbose) {
        throw CommandLineError("verbose is given more than once");
      }
      myIsVerbose = true;
    } else if (!anArgument.empty() && anArgument.front() == '-') {
      throw CommandLineError("unknown option " + std::string(anArgument));
    } else {
      if (hasInput) {
        throw CommandLineError("expected one input file, got '" + myInputPath.string() + "' and '"
                               + std::string(anArgument) + "'");
      }
      myInputPath = std::filesystem::path(anArgument);
      hasInput    = true;
    }
  }

  if (!hasInput) {
    throw CommandLineError("missing input file");
  }
}

std::string CommandLine::Usage() const {
  return "usage: " + myProgramName
         + " <file> [-t|--tolerance <tolerance>] [-o|--output <file>] [-v|--verbose] [-h|--help]\n";
}

std::string CommandLine::Help() const {
  return Usage() +
    "\n"
    "Finds the mirror axes of the convex polygon whose vertices are listed in <file>, one \"X Y\" per line.\n"
    "\n"
    "options:\n"
    "  -t, --tolerance <tolerance>  largest distance allowed between a reflected vertex and its partner\n"
    "                               (default 1e-9)\n"
    "  -o, --output <file>          write the axes to <file> instead of standard output\n"
    "  -v, --verbose                write each axis with its status, anchor point kinds and deviation\n"
    "  -h, --help                   show this help\n"
    "\n"
    "output: one axis per line, given by the two points where it meets the polygon boundary:\n"
    "  <x> <y> - <x> <y>\n"
    "\n"
    "verbose output (-v): one axis per line, columns separated by spaces:\n"
    "  <status> <kind> <index> X=<x> Y=<y> <kind> <index> X=<x> Y=<y> <deviation>\n"
    "  status     verified (proven) or ambiguous (neither proven nor disproven)\n"
    "  kind       point (the axis passes through an input point) or edge (through the midpoint of an edge)\n"
    "  index      0-based index of the point among the points read (comments and blank lines not counted);\n"
    "             for an edge, its two points as i-j\n"
    "  x y        where the axis meets the polygon boundary\n"
    "  deviation  largest distance found between a reflected vertex and its partner\n"
    "\n"
    "In both formats a polygon with no mirror axis gives the single line: non-symmetric\n"
    "\n"
    "exit codes:\n"
    "  0  success   1  usage error   2  input file error   3  not a convex polygon, or coordinates too large\n"
    "  4  output file error   5  internal error\n";
}

}
