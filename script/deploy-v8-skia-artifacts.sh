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

lib_directory='./build/lib'

v8_build_type='release.shared'

for arg in "$@"; do
    case $arg in
        '--use-debug-v8')
            v8_build_type='debug.shared'
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
    'skia/out/Shared/libskia.so'
    'skia/out/Shared/libskottie.so'
    'skia/out/Shared/libskparagraph.so'
    'skia/out/Shared/libsksg.so'
    'skia/out/Shared/libskshaper.so'
    'skia/out/Shared/libsktext.so'
    'skia/out/Shared/libskunicode.so'
)

v8_resource_files=(
    "v8/out/${v8_build_type}/snapshot_blob.bin"
)

for so_file in ${so_library_files[@]}; do
    copy_file ${so_file} ${lib_directory}
done

for res_file in ${v8_resource_files}; do
    copy_file ${res_file} ../qresource/org.cocoa.internal.v8
done

