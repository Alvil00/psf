#include "psf/CommandLine.hxx"

#include <gtest/gtest.h>

#include <string>
#include <vector>


namespace {

using psf::CommandLine;
using psf::CommandLineError;

//! Parses theWords as main() would receive them after the program name "psf".
CommandLine Parse(std::vector<std::string> theWords) {
  theWords.insert(theWords.begin(), "psf");
  std::vector<char*> anArgv;
  for (auto& aWord : theWords) {
    anArgv.push_back(aWord.data());
  }
  return CommandLine(static_cast<int>(anArgv.size()), anArgv.data());
}

}


TEST(CommandLineTest, InputOnlyUsesDefaults) {
  const CommandLine aLine = Parse({"polygon.txt"});
  EXPECT_FALSE(aLine.IsHelp());
  EXPECT_EQ(aLine.InputPath(), "polygon.txt");
  EXPECT_FALSE(aLine.OutputPath());
  EXPECT_FALSE(aLine.IsVerbose());
  EXPECT_EQ(aLine.Tolerance(), psf::BaseTraits::Precision);
}

TEST(CommandLineTest, ReadsLongAndShortOptions) {
  const CommandLine aLong = Parse({"--tolerance", "1e-6", "in.txt", "--output", "out.txt"});
  EXPECT_EQ(aLong.Tolerance(), 1e-6);
  EXPECT_EQ(aLong.InputPath(), "in.txt");
  ASSERT_TRUE(aLong.OutputPath());
  EXPECT_EQ(*aLong.OutputPath(), "out.txt");

  const CommandLine aShort = Parse({"-o", "out.txt", "-t", "0.5", "in.txt"});
  EXPECT_EQ(aShort.Tolerance(), 0.5);
  EXPECT_EQ(*aShort.OutputPath(), "out.txt");
}

TEST(CommandLineTest, ReadsVerboseFlag) {
  EXPECT_TRUE(Parse({"in.txt", "-v"}).IsVerbose());
  EXPECT_TRUE(Parse({"--verbose", "in.txt"}).IsVerbose());

  const CommandLine aLine = Parse({"-v", "-o", "out.txt", "in.txt"});
  EXPECT_TRUE(aLine.IsVerbose());
  EXPECT_EQ(aLine.InputPath(), "in.txt");
  EXPECT_EQ(*aLine.OutputPath(), "out.txt");
}

TEST(CommandLineTest, AcceptsZeroTolerance) {
  EXPECT_EQ(Parse({"in.txt", "-t", "0"}).Tolerance(), 0.0);
}

TEST(CommandLineTest, HelpWinsOverEverythingElse) {
  EXPECT_TRUE(Parse({"-h"}).IsHelp());
  EXPECT_TRUE(Parse({"in.txt", "--bogus", "--help"}).IsHelp());
}

TEST(CommandLineTest, RejectsMissingOrExtraInput) {
  EXPECT_THROW(Parse({}), CommandLineError);
  EXPECT_THROW(CommandLine(0, nullptr), CommandLineError);  // argc 0: not even a program name
  EXPECT_THROW(Parse({"-t", "1"}), CommandLineError);
  EXPECT_THROW(Parse({"a.txt", "b.txt"}), CommandLineError);
}

TEST(CommandLineTest, RejectsUnknownAndRepeatedOptions) {
  EXPECT_THROW(Parse({"in.txt", "--quiet"}), CommandLineError);
  EXPECT_THROW(Parse({"in.txt", "-t", "1", "--tolerance", "2"}), CommandLineError);
  EXPECT_THROW(Parse({"in.txt", "-o", "a", "-o", "b"}), CommandLineError);
  EXPECT_THROW(Parse({"in.txt", "-v", "--verbose"}), CommandLineError);
}

TEST(CommandLineTest, RejectsOptionWithoutValue) {
  EXPECT_THROW(Parse({"in.txt", "-t"}), CommandLineError);
  EXPECT_THROW(Parse({"in.txt", "--output"}), CommandLineError);
}

TEST(CommandLineTest, RejectsBadTolerance) {
  for (const char* aText : {"abc", "1e-3x", "", "-1", "nan", "inf", "+1"}) {
    EXPECT_THROW(Parse({"in.txt", "-t", aText}), CommandLineError) << "'" << aText << "'";
  }
}

TEST(CommandLineTest, UsageAndHelpNameTheProgram) {
  char  aPath[]  = "/usr/local/bin/mirror";
  char  aHelp[]  = "-h";
  char* anArgv[] = {aPath, aHelp};

  const CommandLine aLine(2, anArgv);
  EXPECT_EQ(aLine.ProgramName(), "mirror");
  EXPECT_EQ(aLine.Usage().rfind("usage: mirror <file>", 0), 0u);
  EXPECT_EQ(aLine.Help().rfind(aLine.Usage(), 0), 0u);
}

TEST(CommandLineTest, ErrorCarriesUsage) {
  char  aPath[]  = "./mirror";
  char* anArgv[] = {aPath};
  try {
    CommandLine(1, anArgv);
    FAIL() << "missing input file must throw";
  } catch (const CommandLineError& anError) {
    EXPECT_EQ(anError.Usage().rfind("usage: mirror <file>", 0), 0u);
  }

  try {
    CommandLine(0, nullptr);
    FAIL() << "argc 0 must throw";
  } catch (const CommandLineError& anError) {
    EXPECT_EQ(anError.Usage().rfind("usage: psf <file>", 0), 0u);
  }
}
