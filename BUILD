cc_test(
  name = "test_cppcopyfile",
  size = "medium",
  srcs = glob(["test/*.cc", "src/*.hh", "src/*.cc"]),
  deps = ["@com_google_googletest//:gtest_main"],
  linkopts= ["-lrt", "-pthread"],
)

cc_binary(
  name = "ipc_sendfile",
  srcs = ["ipc_sendfile.cc", "src/parse_args.hh", "src/ipc.hh", "src/ipc.cc", "src/ipcsender.hh", "src/ipcsender.cc"],
  linkopts= ["-lrt", "-pthread"],
)

cc_binary(
  name = "ipc_receivefile",
  srcs = ["ipc_receivefile.cc", "src/parse_args.hh", "src/ipc.hh", "src/ipc.cc", "src/ipcreceiver.hh", "src/ipcreceiver.cc"],
  linkopts= ["-lrt", "-pthread"],
)
