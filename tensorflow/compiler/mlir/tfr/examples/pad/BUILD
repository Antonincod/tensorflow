load("//tensorflow:tensorflow.default.bzl", "tf_py_test")
load("//tensorflow/compiler/mlir/tfr:build_defs.bzl", "gen_op_libraries")

package(
    # copybara:uncomment default_applicable_licenses = ["//tensorflow:license"],
    default_visibility = [
        ":friends",
    ],
    licenses = ["notice"],
)

package_group(
    name = "friends",
    packages = [
        "//tensorflow/compiler/mlir/tfr/...",
    ],
)

gen_op_libraries(
    name = "pad_ops",
    src = "ops_defs.py",
    deps = [
        "//tensorflow:tensorflow_py",
    ],
)

tf_py_test(
    name = "pad_ops_test",
    size = "small",
    srcs = ["pad_ops_test.py"],
    data = [":pad_ops_mlir"],
    python_version = "PY3",
    srcs_version = "PY3",
    tags = [
        "no_windows",  # TODO(b/170752141)
        "nomac",  # TODO(b/170752141)
    ],
    deps = [
        ":pad_ops",
        ":pad_ops_py",
        "//tensorflow:tensorflow_py",
        "//tensorflow/compiler/mlir/tfr:test_utils",
    ],
)
