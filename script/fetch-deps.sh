#!/usr/bin/env bash

NON_GOOGLE_DEPS_LIST=(
    "jsoncpp.tar.gz@https://codeload.github.com/open-source-parsers/jsoncpp/tar.gz/refs/tags/1.9.5"
    "jemalloc.tar.gz@https://codeload.github.com/jemalloc/jemalloc/tar.gz/refs/tags/5.3.0"
    "fmt.tar.gz@https://codeload.github.com/fmtlib/fmt/tar.gz/refs/tags/9.1.0"
    "libunwind.tar.gz@https://codeload.github.com/libunwind/libunwind/tar.gz/refs/tags/v1.6.2"
    "libuv.tar.gz@https://codeload.github.com/libuv/libuv/tar.gz/refs/tags/v1.44.2"
    "libsquash.tar.gz@https://codeload.github.com/pmq20/libsquash/tar.gz/refs/tags/v0.8.0"
    "ffmpeg.tar.gz@http://www.ffmpeg.org/releases/ffmpeg-6.0.tar.gz"
    "libwebsockets.tar.gz@https://codeload.github.com/warmcat/libwebsockets/tar.gz/refs/tags/v4.3.2"
)

GOOGLE_V8_CHECKOUT_VER=11.1

PACKAGES_DIR="./third_party/packages"
BUILD_DIR="./third_party/build"

source ./script/colored-output.sh
if [[ ! -d ${PACKAGES_DIR} ]]; then
    print_info "Creating directory ${PACKAGES_DIR}"
    mkdir ${PACKAGES_DIR}
fi

if [[ ! -d ${BUILD_DIR} ]]; then
    print_info "Creating directory ${BUILD_DIR}"
    mkdir ${BUILD_DIR}
fi

# Download packages
print_info "Fetching non-google dependencies..."
for pkg_descriptor in ${NON_GOOGLE_DEPS_LIST[@]}; do
    print_info "Checking dependency \033[34;1m${pkg_descriptor}\033[0m"
    dep_filename=$(echo $pkg_descriptor | cut -d @ -f 1)
    dep_url=$(echo $pkg_descriptor | cut -d @ -f 2)
    if [[ -f "${PACKAGES_DIR}/${dep_filename}" ]]; then
        print_info "Package already exists, skipped"
        continue
    fi

    # Fetch the package
    print_info "Fetching ${dep_filename} from ${dep_url}..."
    if ! curl "${dep_url}" --output ${PACKAGES_DIR}/${dep_filename}; then
        print_err "Failed to fetch ${dep_filename} from ${dep_url}"
        exit 1
    fi
done

# Extract packages
for pkg_descriptor in ${NON_GOOGLE_DEPS_LIST[@]}; do
    pkg_filename=$(echo $pkg_descriptor | cut -d @ -f 1)
    pkg_path="${PACKAGES_DIR}/${pkg_filename}"
    pkg_name=$(echo $pkg_filename | cut -d . -f 1)
    print_info "Extracting package ${pkg_filename}..."

    if [[ -n $(find ./third_party -maxdepth 1 -type d -name "${pkg_name}-*") ]]; then
        print_info "Extracted package already exists, skipped"
        continue
    fi

    if ! tar -C ./third_party -xzf ${pkg_path}; then
        print_err "Failed to extract ${dep_path}"
        exit 1
    fi
done

function setup_google_depot_tools() {
    if [[ -d "./third_party/depot_tools" ]]; then
        return 0
    fi
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
    if ! $?; then
        print_err "Failed to clone Google depot_tools repository"
        exit 1
    fi
    export PATH=${PATH}:./third_party/depot_tools

    print_info "Update depot_tools..."
    gclient
}

function try_fetch_google_dep() {
    pkgname=$1
    print_info "Try fetching Google dependency ${pkgname}..."
    if [[ -d "./third_party/${pkgname}" ]]; then
        print_info "Extracted package directory found, skipped"
        return 0
    fi

    setup_google_depot_tools
    if ! fetch $pkgname; then
        print_err "Failed to fetch Google dependency ${pkgname}"
        exit 1
    fi
}

try_fetch_google_dep skia
try_fetch_google_dep v8

# Check v8
cd ./third_party/v8
if [[ $(git branch --show-current) != ${GOOGLE_V8_CHECKOUT_VER} ]]; then
    print_info "Checkout v8 version ${GOOGLE_V8_CHECKOUT_VER}"
    git checkout -b ${GOOGLE_V8_CHECKOUT_VER} -t branch-heads/${GOOGLE_V8_CHECKOUT_VER}
fi
