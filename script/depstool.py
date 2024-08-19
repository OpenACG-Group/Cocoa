#!/usr/bin/env python

# This file is part of Cocoa.
#
# Cocoa is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# Cocoa is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Cocoa. If not, see <https://www.gnu.org/licenses/>.

import os
import sys
import argparse
import subprocess
from typing import Literal

BUILD_SYSTEM_CMAKE = 'cmake'
BUILD_SYSTEM_GN = 'gn'
BUILD_SYSTEM_AUTOTOOLS = 'autotools'

TOOLCHAIN_NINJA = '/usr/bin/ninja'
TOOLCHAIN_MAKE = '/usr/bin/make'
TOOLCHAIN_CMAKE = '/usr/bin/cmake'
TOOLCHAIN_GN = '/usr/bin/gn'
TOOLCHAIN_C_COMPILER = '/usr/bin/clang'
TOOLCHAIN_CXX_COMPILER = '/usr/bin/clang++'


class ComponentBase:
    def __init__(self, name: str, build_system: str):
        self.name = name
        self.build_system = build_system

    def fetch(self, target_dir: str):
        pass

    def build(self, build_type: Literal['release', 'debug'], install_dir: str,
              c_compiler: str, cxx_compiler: str) -> None:
        raise NotImplementedError('build system not implemented')

    def check_skip_or_rebuild(self, build_artifact_dir: str) -> str:
        if os.path.exists(build_artifact_dir):
            choice = input(f'Previously existing build at `{build_artifact_dir}`, skip or rebuild? [SKIP/rebuild/partial] ')
            if len(choice) == 0 or choice.casefold() == 'skip'.casefold():
                print(f'Skip building package {self.name}')
                return 'skip'
            elif choice.casefold() == 'rebuild'.casefold():
                print(f'Rebuild package {self.name}')
                subprocess.check_call(f'rm -r {build_artifact_dir}', shell=True)
            elif choice.casefold() == 'partial'.casefold():
                print(f'Partial (incremental) rebuild package {self.name}')
                return 'partial'
            else:
                # Using a loop to handle the wrong input (ask the user to retry) would be a better idea,
                # but, well, this is just a simple tool.
                raise RuntimeError(f'unrecognized choice: {choice}')

        os.mkdir(build_artifact_dir)
        return 'rebuild'


class CmakeComponentBase(ComponentBase):
    def __init__(self, name: str):
        super().__init__(name, BUILD_SYSTEM_CMAKE)

    def build(self, build_type: Literal['release', 'debug'], install_dir: str,
              c_compiler: str, cxx_compiler: str) -> None:

        self.on_prepare()

        # Construct cmake commandline arguments
        cmake_cmd_args = [
            TOOLCHAIN_CMAKE,
            '-G', 'Ninja',
            f'-DCMAKE_INSTALL_PREFIX={install_dir}',
            f'-DCMAKE_CXX_COMPILER={cxx_compiler}',
            f'-DCMAKE_C_COMPILER={c_compiler}',
        ]
        if build_type == 'release':
            cmake_cmd_args.append('-DCMAKE_BUILD_TYPE=Release')
        elif build_type == 'debug':
            cmake_cmd_args.append('-DCMAKE_BUILD_TYPE=Debug')
        else:
            assert False, 'unrecognized build type'

        cmake_cmd_args.extend(self.on_cmake_extra_flags())

        # Create `//third_party/<package>/out` directory for building
        if not os.path.exists(self.name):
            raise RuntimeError(f'Package directory `{os.getcwd()}/third_party/{self.name}` not found')

        build_artifact_dir = f'{self.name}/out'

        rebuild_type = self.check_skip_or_rebuild(build_artifact_dir)
        if rebuild_type == 'skip':
            return

        # Partial rebuild skips configuration
        if rebuild_type == 'rebuild':
            # Start cmake configuration
            os.chdir(build_artifact_dir)
            cmake_cmd_args.append('..')
            subprocess.check_call(cmake_cmd_args)
            os.chdir('../..')

            self.on_post_cmake()

        # Build and install
        subprocess.check_call([TOOLCHAIN_NINJA, '-C', build_artifact_dir, 'install'])
        self.on_post_build()

    def on_prepare(self) -> None:
        pass

    def on_post_cmake(self) -> None:
        pass

    def on_post_build(self) -> None:
        pass

    def on_cmake_extra_flags(self) -> list[str]:
        return []


class LibuvComponent(CmakeComponentBase):
    def __init__(self):
        super().__init__('libuv')


class JsonCppComponent(CmakeComponentBase):
    def __init__(self):
        super().__init__('jsoncpp')


class LibwebsocketsComponent(CmakeComponentBase):
    def __init__(self):
        super().__init__('libwebsockets')

    def on_cmake_extra_flags(self) -> list[str]:
        return [
            '-DLWS_WITH_STATIC=OFF',
            '-DLWS_WITH_JSONRPC=OFF',
            '-DLWS_WITH_TESTAPPS=OFF',
            '-DLWS_LINK_TESTAPPS_DYNAMIC=ON',
            '-DLWS_WITH_LIBUV=ON'
        ]


class FmtComponent(CmakeComponentBase):
    def __init__(self):
        super().__init__('fmt')


class LibyuvComponent(CmakeComponentBase):
    def __init__(self):
        super().__init__('libyuv')


class SkiaComponent(ComponentBase):
    def __init__(self):
        super().__init__('skia', BUILD_SYSTEM_GN)

    def build(self, build_type: Literal['release', 'debug'], install_dir: str,
              c_compiler: str, cxx_compiler: str) -> None:

        if not os.path.exists('skia'):
            raise RuntimeError('Package directory //third_party/skia not found')

        rebuild_type = self.check_skip_or_rebuild('skia/out')
        if rebuild_type == 'skip':
            return

        if rebuild_type == 'rebuild':
            # Generate GN command line
            gn_gen_args = 'is_component_build=true skia_use_vulkan=true skia_use_gl=false '
            if build_type == 'release':
                gn_gen_args += 'is_official_build=true is_debug=false '
            elif build_type == 'debug':
                gn_gen_args += 'is_official_build=false is_debug=true '
            else:
                assert False, 'unrecognized build type'

            gn_gen_args += f'cc="{c_compiler}" cxx="{cxx_compiler}" ' \
                           + 'skia_use_icu=true skia_use_client_icu=false skia_use_system_icu=false ' \
                           + 'skia_use_system_harfbuzz=true skia_disable_tracing=false ' \
                           + 'skia_enable_fontmgr_FontConfigInterface=false skia_enable_fontmgr_android=false ' \
                           + 'skia_enable_fontmgr_fontconfig=true'

            # Start gn configuration
            os.chdir('skia')
            subprocess.check_call([
                TOOLCHAIN_GN,
                'gen',
                'out',
                f'--args={gn_gen_args}'
            ])
            os.chdir('..')

        # Start to build
        subprocess.check_call([
            TOOLCHAIN_NINJA,
            '-C',
            'skia/out'
        ])

        # Copy files to destination directory
        install_lib_dir = f'{install_dir}/lib'
        if not os.path.exists(install_lib_dir):
            os.mkdir(install_lib_dir)

        print(f'Copying Skia libraries to {install_lib_dir}')
        subprocess.check_call([
            '/usr/bin/cp',
            'skia/out/libskia.so',
            'skia/out/libskparagraph.so',
            'skia/out/libskshaper.so',
            'skia/out/libskunicode.so',
            install_lib_dir
        ])


class V8Component(ComponentBase):
    def __init__(self):
        super().__init__('v8', BUILD_SYSTEM_GN)

    def build(self, build_type: Literal['release', 'debug'], install_dir: str,
              c_compiler: str, cxx_compiler: str) -> None:

        if not os.path.exists('v8'):
            raise RuntimeError('Package directory //third_party/v8 not found')

        rebuild_type = self.check_skip_or_rebuild('v8/out')
        if rebuild_type == 'skip':
            return

        # Partial rebuild skips this
        if rebuild_type == 'rebuild':
            # Generate GN command line
            gn_gen_args = 'use_custom_libcxx=false is_component_build=true v8_use_external_startup_data=false ' \
                          + 'v8_enable_sandbox=false v8_enable_disassembler=true v8_enable_object_print=true ' \
                          + 'v8_enable_backtrace=true'
            if build_type == 'release':
                gn_gen_args += 'is_debug=false'
            elif build_type == 'debug':
                gn_gen_args += 'is_debug=true'
            else:
                assert False, 'unrecognized build type'

            # Start GN configuration
            os.chdir('v8')
            subprocess.check_call([
                TOOLCHAIN_GN,
                'gen',
                'out',
                f'--args={gn_gen_args}'
            ])

            os.chdir('..')

        # Start to build
        subprocess.check_call([
            TOOLCHAIN_NINJA,
            '-C',
            'v8/out'
        ])

        # Copy files to destination directory
        install_lib_dir = f'{install_dir}/lib'
        if not os.path.exists(install_lib_dir):
            os.mkdir(install_lib_dir)

        print(f'Copying Skia libraries to {install_lib_dir}')
        subprocess.check_call([
            '/usr/bin/cp',
            'v8/out/libcppgc.so',
            'v8/out/libicuuc.so',
            'v8/out/libv8.so',
            'v8/out/libv8_libbase.so',
            'v8/out/libv8_libplatform.so',
            'v8/out/libchrome_zlib.so',
            'v8/out/libthird_party_icu_icui18n.so',
            'v8/out/libthird_party_abseil-cpp_absl.so',
            install_lib_dir
        ])


class FFmpegComponent(ComponentBase):
    def __init__(self):
        super().__init__('ffmpeg', BUILD_SYSTEM_AUTOTOOLS)

    def build(self, build_type: Literal['release', 'debug'], install_dir: str,
              c_compiler: str, cxx_compiler: str) -> None:

        if build_type == 'debug':
            raise RuntimeError('We does not support building ffmpeg in debug mode')

        if not os.path.exists('ffmpeg'):
            raise RuntimeError('Package directory //third_party/ffmpeg not found')

        rebuild_type = self.check_skip_or_rebuild('ffmpeg/out')
        if rebuild_type == 'skip':
            return

        # Partial rebuild skips this
        if rebuild_type == 'rebuild':
            os.chdir('ffmpeg/out')

            # We ignore `c_compiler` and `cxx_compiler` arguments since they are not
            # fully tested.
            subprocess.check_call([
                '../configure',
                f'--prefix={install_dir}',
                '--enable-gpl',
                '--enable-version3',
                '--disable-everything',
                '--disable-all',
                '--disable-static',
                '--enable-avcodec',
                '--enable-avformat',
                '--enable-avutil',
                '--enable-swresample',
                '--enable-libopus',
                '--enable-shared',
                '--enable-hwaccels',
                '--enable-vaapi',
                '--disable-bzlib',
                '--disable-iconv',
                '--disable-network',
                '--disable-schannel',
                '--disable-sdl2',
                '--disable-symver',
                '--disable-xlib',
                '--disable-zlib',
                '--disable-securetransport',
                '--disable-faan',
                '--disable-alsa',
                '--disable-autodetect',
                '--enable-decoder=vorbis,libopus,flac,aac,h264,mjpeg,png,inflate',
                '--enable-decoder=pcm_u8,pcm_s16le,pcm_s24le,pcm_s32le,pcm_f32le,mp3,ac3',
                '--enable-decoder=pcm_s16be,pcm_s24be,pcm_mulaw,pcm_alaw',
                '--enable-demuxer=ogg,matroska,wav,flac,mp3,mov,flv,aac',
                '--enable-parser=opus,vorbis,flac,mpegaudio,vp9,webp,mjpeg,gif,png,aac,h264',
                '--disable-linux-perf',
                '--enable-lto',
                '--enable-pic',
                '--enable-avfilter',
                '--enable-filters',
                '--enable-swscale',
                '--enable-zlib',
                '--enable-protocols'
            ])
            os.chdir('../..')

        # Build and install
        os.chdir('ffmpeg/out')
        subprocess.check_call([
            TOOLCHAIN_MAKE,
            'install'
        ])
        os.chdir('../..')


PACKAGES_CLASS_MAP = {
    'libuv': LibuvComponent,
    'jsoncpp': JsonCppComponent,
    'libwebsockets': LibwebsocketsComponent,
    'fmt': FmtComponent,
    'libyuv': LibyuvComponent,
    'skia': SkiaComponent,
    'v8': V8Component,
    'ffmpeg': FFmpegComponent
}


def subcommand_build(packages: list[str], args: argparse.Namespace):
    install_dir = f'{os.getcwd()}/build'
    if not os.path.exists(install_dir):
        print('Create install directory `//third_party/build`')
        os.mkdir(install_dir)

    for pkg_name in packages:
        print(f'Building package {pkg_name}')
        PACKAGES_CLASS_MAP[pkg_name]().build(args.build_type, install_dir, TOOLCHAIN_C_COMPILER, TOOLCHAIN_CXX_COMPILER)


def run():
    if not os.path.exists('./PROJECTROOT'):
        print('You must run depstool in the project root directory')
        sys.exit(1)

    os.chdir('./third_party')

    args_parser = argparse.ArgumentParser()
    args_parser.add_argument('subcommand', choices=['fetch', 'build', 'list'])
    args_parser.add_argument('packages', nargs='*')
    args_parser.add_argument('--build-type', choices=['release', 'debug'], default='release')
    args = args_parser.parse_args()

    if args.subcommand == 'fetch':
        # TODO(sora): implement this.
        raise NotImplementedError('not implemented yet')
    elif args.subcommand == 'build':
        subcommand_build(args.packages, args)
    elif args.subcommand == 'list':
        for name, component in PACKAGES_CLASS_MAP.items():
            build_system = component().build_system
            print(f'Package {name}: build_system={build_system}')

    else:
        raise RuntimeError(f'unrecognized subcommand {args.verb}')


if __name__ == '__main__':
    run()
