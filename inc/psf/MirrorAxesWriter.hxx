#pragma once

#include "psf/MirrorAxesWriterError.hxx"
#include "psf/MirrorAxis.hxx"

#include <filesystem>
#include <ostream>
#include <vector>


namespace psf {

//! The text format MirrorAxesWriter writes each axis in.
enum class OutputFormat {
  Plain,   //!< the two axis anchor points only: "<x> <y> - <x> <y>"
  Verbose  //!< "<status> <kind> <index> X=<x> Y=<y> <kind> <index> X=<x> Y=<y> <deviation>"
};

//! Writes mirror axes one per line, in the OutputFormat given:
//!   Plain:   <x> <y> - <x> <y>
//!   Verbose: <status> <kind> <index> X=<x> Y=<y> <kind> <index> X=<x> Y=<y> <deviation>
//! status is "verified" or "ambiguous"; kind is "point" with the input point index, or "edge" with the indices of the
//! edge's two input points as "i-j". Numbers use the shortest text that reads back as the same double; -0 prints as 0.
//! With no axes, both formats write the single line "non-symmetric".
class MirrorAxesWriter {


public:

  MirrorAxesWriter() = delete;

  //! Writes theAxes to theStream in theFormat and flushes it. Throws MirrorAxesWriterError if the stream fails.
  static void Write(std::ostream& theStream, const std::vector<MirrorAxis>& theAxes, OutputFormat theFormat);

  //! Writes theAxes to the file at thePath in theFormat, replacing its contents.
  //! Throws MirrorAxesWriterError; its what() is prefixed with the path.
  static void Write(const std::filesystem::path& thePath, const std::vector<MirrorAxis>& theAxes,
                    OutputFormat theFormat);

};


}
