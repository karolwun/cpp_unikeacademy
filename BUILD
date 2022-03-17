cc_test(
  name = "test_cppcopyfile",
  size = "small",
  srcs = ["test_cppcopyfile.cc", "src/parse_args.hh", "src/ipc.hh"],
  deps = ["@com_google_googletest//:gtest_main"],
)

cc_binary(
  name = "ipc_sendfile",
  srcs = ["ipc_sendfile.cc", "src/parse_args.hh", "src/ipc.hh", "src/ipcsender.cc"],
  linkopts= ["-lrt", "-pthread"],
)

cc_binary(
  name = "ipc_receivefile",
  srcs = ["ipc_receivefile.cc", "src/parse_args.hh", "src/ipc.hh", "src/ipcreceiver.cc"],
  linkopts= ["-lrt", "-pthread"],
)
