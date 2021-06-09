import os
import sys
import subprocess
import termcolor
import script.cocoaproject

_CR = termcolor.colored
task_components = []
help_information = '''
BUILD.py: Python Building Script for Cocoa
Cocoa Project, version ${}
'''


def message(msg):
    print(_CR("## ", color="white", attrs=["bold"]) + _CR(msg, color="green"))


def execute(args, env=None):
    absolute_path = None
    if os.access(args[0], os.F_OK):
        absolute_path = os.path.abspath(args[0])
    else:
        path_list = os.getenv("PATH").split(':')
        for path in path_list:
            if os.access(path + "/" + args[0], os.X_OK):
                absolute_path = path + "/" + args[0]
                break
        if absolute_path is None:
            raise PermissionError("execute: Command not found: " + args[0])
    args[0] = absolute_path
    return subprocess.call(args=args, stdin=None, stdout=None, shell=False, env=env)


def mkdir_if_not_found(dir):
    if not os.path.exists(dir):
        os.mkdir(dir)


def cmake_configure(home_dir, build_dir, options, backend="Ninja"):
    args = ["cmake", "-Wno-dev", "-G", backend]
    for opt in options:
        args.append("-D" + opt)
    args.append(os.path.abspath(home_dir))
    current_dir = os.path.abspath(os.path.curdir)
    os.chdir(build_dir)
    ret = execute(args=args, env=os.environ)
    os.chdir(current_dir)
    return ret


def ninja_build(build_dir):
    return execute(args=["ninja", "-C", build_dir], env=os.environ)


def build_poco_static_cmake():
    poco_cmake_options = ["CMAKE_C_COMPILER=clang",
                          "CMAKE_CXX_COMPILER=clang++",
                          "CMAKE_CXX_FLAGS=-Wno-implicit-const-int-float-conversion",
                          "BUILD_SHARED_LIBS=off",
                          "ENABLE_JWT=off",
                          "ENABLE_DATA=off",
                          "ENABLE_DATA_SQLITE=off",
                          "ENABLE_MONGODB=off",
                          "ENABLE_REDIS=off",
                          "ENABLE_PDF=off",
                          "ENABLE_ZIP=off",
                          "ENABLE_SEVENZIP=off",
                          "ENABLE_PAGECOMPILER=off",
                          "ENABLE_PAGECOMPILER_FILE2PAGE=off"]
    mkdir_if_not_found("third_party/poco/out")
    ret = cmake_configure("third_party/poco", "third_party/poco/out", poco_cmake_options)
    if ret != 0:
        raise RuntimeError("Failed to configure Poco library (Building system: CMake)")
    ninja_build("third_party/poco/out")


def build_skia_shared_gn():
    skia_gn_options = ["is_component_build=true",
                       "skia_use_vulkan=true",
                       "skia_use_gl=false",
                       "is_official_build=true",
                       "cc=\"clang\"",
                       "cxx=\"clang++\""]
    current_dir = os.path.abspath('.')
    os.chdir("third_party/skia")
    gn_args = ""
    for k in skia_gn_options:
        gn_args += k + ' '
    ret = execute(args=["./bin/gn", "gen", "out/Shared", "--args=" + gn_args], env=os.environ)
    if ret != 0:
        raise RuntimeError("Failed to configure Skia library (Building system: GN)")

    os.chdir(current_dir)


def parse_command_list(param: str):
    return param.split(',')


def parse_command_args():
    i = 0
    while i < len(sys.argv):
        if sys.argv[i] == "--help":

        elif sys.argv[i] == "--components":
            task_components = sys.argv[++i].split(',')
        else:
            raise RuntimeError("Unknown command option: " + sys.argv[i])
        i += 1


# Build dependencies
message("Building dependency: Poco static library...")
build_poco_static_cmake()
message("Building dependency: Skia shared library...")
build_skia_shared_gn()
