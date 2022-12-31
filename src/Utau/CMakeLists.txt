find_package(PipeWire REQUIRED)

add_library(
        Utau

        Utau.h
        Utau.cc

        AVStreamDecoder.h
        AVStreamDecoder.cc

        AVGenericBuffer.h
        AVGenericBuffer.cc
        AudioBufferInfo.h
        AudioBuffer.h
        AudioBuffer.cc
        AudioSink.h
        AudioSink.cc
        PipeWireAudioSink.h
        PipeWireAudioSink.cc
        AudioFilterDAG.h
        AudioFilterDAG.cc

        AudioPlaybackContext.h

        ffwrappers/libavfilter.h
        ffwrappers/libavutil.h
        ffwrappers/libavformat.h
        ffwrappers/libavcodec.h
        ffwrappers/libswresample.h
        ffwrappers/samplefmt.h
)

target_link_libraries(Utau PUBLIC PipeWire::PipeWire avutil avformat avcodec avfilter)
