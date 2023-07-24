find_package(PipeWire REQUIRED)

add_library(
        Utau

        Utau.h
        Utau.cc

        AVStreamDecoder.h
        AVStreamDecoder.cc
        AVFilterDAG.h
        AVFilterDAG.cc

        AVGenericBuffer.h
        AVGenericBuffer.cc
        AudioBufferInfo.h
        AudioBuffer.h
        AudioBuffer.cc
        AudioDevice.h
        AudioSinkStream.h
        AudioSinkStream.cc
        AudioFilterInputSinkStream.h
        AudioFilterInputSinkStream.cc

        VideoBufferInfo.h
        VideoBufferInfo.cc
        VideoBuffer.h
        VideoBuffer.cc

        HWDeviceContext.h
        HWDeviceContext.cc

        VideoFrameGLEmbedder.h
        VideoFrameGLEmbedder.cc

        pipewire/PipeWireAudioDevice.h
        pipewire/PipeWireAudioDevice.cc
        pipewire/PipeWireAudioSinkStream.h
        pipewire/PipeWireAudioSinkStream.cc

        ffwrappers/libavfilter.h
        ffwrappers/libavutil.h
        ffwrappers/libavformat.h
        ffwrappers/libavcodec.h
        ffwrappers/libswresample.h
        ffwrappers/libswscale.h
        ffwrappers/samplefmt.h
        ffwrappers/pixfmt.h
)

target_link_libraries(
        Utau PUBLIC
        PipeWire::PipeWire
        avutil
        avformat
        avcodec
        avfilter
        swscale
        drm
        va
        va-drm
)

target_include_directories(
        Utau PUBLIC
        /usr/include/libdrm
)