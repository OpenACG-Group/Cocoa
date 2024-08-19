find_package(PipeWire REQUIRED)

add_library(
        Utau

        Utau.cc
        PixelFormatUtils.cc
        HWDeviceContext.cc
        FrameTextureConverter.cc
        pipewire/PipeWireAudioDevice.cc
        pipewire/PipeWireAudioSinkStream.cc
)

target_link_libraries(
        Utau PUBLIC
        PipeWire::PipeWire
        avformat
        avcodec
        avfilter
        swscale
        swresample
        avutil
        drm
        va
        va-drm
)

target_include_directories(
        Utau PUBLIC
        /usr/include/libdrm
)