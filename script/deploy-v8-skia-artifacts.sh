#!/usr/bin/env bash

function copy_file() {
    local src_file=$1
    local dst_file=$2
    if [[ ! -f $src_file ]]; then
        echo -e "\e[33;1mMissing file $src_file\e[0m"
        return 1
    fi
    echo -e "Copying \e[32;1m$src_file\e[0m to $dst_file"
    cp $src_file $dst_file
}

build_directory='./build'
lib_directory="${build_directory}/lib"

v8_build_type='release.shared'
skia_build_type='release.shared'

for arg in "$@"; do
    case $arg in
        '--use-debug-v8')
            v8_build_type='debug.shared'
            ;;
        '--use-debug-skia')
            skia_build_type='debug.shared'
            ;;
        *)
            echo "Unrecognized argument $arg"
            ;;
    esac
done

so_library_files=(
    "v8/out/${v8_build_type}/libv8.so"
    "v8/out/${v8_build_type}/libv8_libbase.so"
    "v8/out/${v8_build_type}/libv8_libplatform.so"
    "v8/out/${v8_build_type}/libchrome_zlib.so"
    "v8/out/${v8_build_type}/libicuuc.so"
    "v8/out/${v8_build_type}/libicui18n.so"
    "skia/out/${skia_build_type}/libskia.so"
    "skia/out/${skia_build_type}/libskottie.so"
    "skia/out/${skia_build_type}/libskparagraph.so"
    "skia/out/${skia_build_type}/libsksg.so"
    "skia/out/${skia_build_type}/libskshaper.so"
    "skia/out/${skia_build_type}/libsktext.so"
    "skia/out/${skia_build_type}/libskunicode.so"
)

resource_files=(
    "v8/out/${v8_build_type}/snapshot_blob.bin"
)

for so_file in ${so_library_files[@]}; do
    copy_file ${so_file} ${lib_directory}
done

for res_file in ${resource_files[@]}; do
    copy_file ${res_file} ${build_directory}
done

