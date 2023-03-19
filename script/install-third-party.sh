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
skia_shared_build_type='release.shared'
skia_static_build_type='release.static'

for arg in "$@"; do
    case $arg in
        '--v8-build-type='*)
            v8_build_type=$(echo $arg | cut -d '=' -f 2)
            ;;
        '--skia-shared-build-type='*)
            skia_build_type=$(echo $arg | cut -d '=' -f 2)
            ;;
        '--skia-static-build-type='*)
            skia_static_build_type=$(echo $arg | cut -d '=' -f 2)
            ;;
        *)
            echo "Unrecognized argument $arg"
            ;;
    esac
done

v8_shared_libs=(
    "v8/out/${v8_build_type}/lib_v8.so"
    "v8/out/${v8_build_type}/lib_v8_libbase.so"
    "v8/out/${v8_build_type}/lib_v8_libplatform.so"
    "v8/out/${v8_build_type}/libchrome_zlib.so"
    "v8/out/${v8_build_type}/libicuuc.so"
    "v8/out/${v8_build_type}/libthird_party_icu_icui18n.so"
)

skia_shared_libs=(
    "skia/out/${skia_shared_build_type}/libskia.so"
    "skia/out/${skia_shared_build_type}/libskottie.so"
    "skia/out/${skia_shared_build_type}/libskparagraph.so"
    "skia/out/${skia_shared_build_type}/libsksg.so"
    "skia/out/${skia_shared_build_type}/libskshaper.so"
    "skia/out/${skia_shared_build_type}/libsktext.so"
    "skia/out/${skia_shared_build_type}/libskunicode.so"
)

skia_static_libs=(
    "skia/out/${skia_static_build_type}/libskottie.a"
    "skia/out/${skia_static_build_type}/libskparagraph.a"
    "skia/out/${skia_static_build_type}/libsksg.a"
    "skia/out/${skia_static_build_type}/libskshaper.a"
    "skia/out/${skia_static_build_type}/libsktext.a"
    "skia/out/${skia_static_build_type}/libskunicode.a"
)

resource_files=()

for so_file in ${v8_shared_libs[@]}; do
    copy_file ${so_file} ${lib_directory}
done

for so_file in ${skia_shared_libs[@]}; do
    copy_file ${so_file} ${lib_directory}
done

for so_file in ${skia_static_libs[@]}; do
    copy_file ${so_file} ${lib_directory}
done

for res_file in ${resource_files[@]}; do
    copy_file ${res_file} ${build_directory}
done
