#!/usr/bin/env python3

import os
import json
import subprocess
import argparse


class PackageLists:
    def __init__(self, d):
        self.__dict__ = d


class GlobalVariables:
    def __init__(self):
        self.third_party_dir = os.getcwd() + "/third_party"
        self.build_dir = os.getcwd() + "/third_party/build"
        self.patch_dir = os.getcwd() + "/third_party/patch"
        self.parallel_count = os.cpu_count()
        self.json_config_path = os.getcwd() + "/third_party/package-lists.json"
        if not os.access(self.json_config_path, os.R_OK):
            raise FileNotFoundError("Couldn't find configuration file " + self.json_config_path)
        json_file = open(self.json_config_path, "r")
        self.config = json.load(json_file, object_hook=PackageLists)
        self.targets = []
        self.force_refetch = False


__global = GlobalVariables()


def execute(args, env=None, stdin=None):
    absolute_path = None
    if os.access(args[0], os.X_OK) is True and os.path.isfile(args[0]):
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
    return subprocess.call(args=args, stdin=stdin, stdout=None, shell=False, env=env)


def parse_fetch(name, fetch, patch):
    path = __global.third_party_dir + "/" + name
    if os.path.exists(path):
        if __global.force_refetch:
            execute(["rm", "-r", path])
        else:
            return

    print("Fetching " + fetch.protocol + " URL: " + fetch.url)
    restore_dir = os.getcwd()
    os.chdir(__global.third_party_dir)
    if fetch.protocol == "git":
        if execute(["git", "clone", fetch.url]) != 0:
            print("Failed to clone repository " + fetch.url)
            os.chdir(restore_dir)
            raise RuntimeError()
        if execute(["git", "submodule", "update", "--init", "--recursive"]) != 0:
            print("Failed to update submodules")
            os.chdir(restore_dir)
            raise RuntimeError()
    else:
        print("Unexpected fetching protocol: " + fetch.protocol)
        os.chdir(restore_dir)
        raise RuntimeError()

    os.chdir(name)
    if patch is not None:
        for p in patch:
            pure_patch_path = variable_replace(p, path)
            patch_file = open(pure_patch_path, "r")
            execute(["patch", "-p0"], stdin=patch_file)
    os.chdir(restore_dir)


def variable_replace(origin: str, source_dir):
    result = origin.replace("${source_dir}", source_dir)\
        .replace("${build_dir}", __global.build_dir)\
        .replace("${parallel_count}", str(__global.parallel_count))\
        .replace("${patch_dir}", __global.patch_dir)
    return result


def parse_compile(name, build_instructions, source_dir):
    print("Building package...")
    restore_path = os.getcwd()
    for inst in build_instructions:
        working_dir = variable_replace(inst.workingDir, source_dir)
        command = [variable_replace(cmd, source_dir) for cmd in inst.command]
        print("Enter " + working_dir + ": " + str(command))
        os.chdir(working_dir)
        execute(command)
        os.chdir(restore_path)


def parse_package(package, n, total):
    source_dir = __global.third_party_dir + "/" + package.name
    print("[" + str(n) + "/" + str(total) + "] Processing package " + source_dir)
    fetch = False
    build = False
    if package.policy == "fetch-only":
        fetch = True
    elif package.policy == "compile":
        fetch = True
        build = True
    else:
        print("Unexpected package.policy field: " + package.policy)
        return

    tempDirs = []
    if hasattr(package, "temporaryDirs"):
        tempDirs = [variable_replace(d, source_dir) for d in package.temporaryDirs]
    for d in tempDirs:
        if not os.path.exists(d):
            os.mkdir(d)

    if hasattr(package, "fetch") and fetch:
        patch = None
        if hasattr(package, "patch"):
            patch = package.patch
        parse_fetch(package.name, package.fetch, patch)
    if hasattr(package, "compile") and build:
        parse_compile(package.name, package.compile, source_dir)

    for d in tempDirs:
        if os.path.exists(d):
            execute(["rm", "-r", d])


def parse_root():
    if __global.config.signature != "cocoa::build_system:package-list":
        print("Invalid signature in package-lists.json.")
    if __global.config.version != "1.0.0":
        print("Invalid version number in package-lists.json")

    print("package-lists: " + __global.config.signature + ", version " + __global.config.version)
    if "all" in __global.targets:
        __global.targets = [p.name for p in __global.config.packages]
    package_count = len(__global.targets)

    print(str(len(__global.targets)) + " package(s) will be processed")
    input("Press enter to continue...")

    idx = 1
    for obj in __global.config.packages:
        if obj.name in __global.targets:
            parse_package(obj, idx, package_count)
            idx += 1


cmd_parser = argparse.ArgumentParser(prog="build-third-party.py",
                                     description="Fetch and build third-party dependencies",
                                     add_help=True)
cmd_parser.add_argument("--targets",
                        type=str,
                        help="Specify which targets will be built",
                        default="")

args = cmd_parser.parse_args()
__global.targets = args.targets.split(',')

parse_root()
