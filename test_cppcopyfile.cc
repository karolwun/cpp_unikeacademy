#include <gtest/gtest.h>

#include "src/parse_args.hh"

TEST(ParserTest, PassedFileOnly) {
  std::vector<std::string> args{"--file", "resd"};
  auto parsed_args = ParseArgs(args);
  EXPECT_EQ(parsed_args.getFile(), std::string("resd"));
  EXPECT_EQ(parsed_args.isHelp(), true);
  EXPECT_EQ(parsed_args.getIPCMethod(), IPCMethod::NONE);
}

