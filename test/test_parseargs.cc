#include "../src/parse_args.hh"

#include <gtest/gtest.h>

using namespace std;

class ParserTestsP : public ::testing::TestWithParam<tuple<vector<string>, bool, 
                                                     string, IPCType, optional<vector<string>>> >
{};

TEST_P(ParserTestsP, CheckParsingArgs) {
    auto cmdline = get<0>(GetParam());
    ParseArgs args(cmdline);

    ASSERT_EQ(args.isHelp(), get<1>(GetParam()));
    ASSERT_EQ(args.getFile(), get<2>(GetParam()));
    ASSERT_EQ(args.getIPCType(), get<3>(GetParam()));
    ASSERT_EQ(args.getParams(), get<4>(GetParam()));    
}

// Parameters: cmdline, isHelp, file, IPCType, params
INSTANTIATE_TEST_CASE_P(ParserTests, ParserTestsP,
                        ::testing::Values(
                            make_tuple(vector<string>{"--file", "k"}, true, "", IPCType::NONE, nullopt),
                            make_tuple(vector<string>{"--pipe"}, true, "", IPCType::NONE, nullopt),
                            make_tuple(vector<string>{"--pipe", "--file", "filename"}, false, "filename", IPCType::pipe, nullopt),
                            make_tuple(vector<string>{"--shm", "4", "--file", "filename"}, false, "filename", IPCType::shmem, vector<string>{"4"})
                        ));