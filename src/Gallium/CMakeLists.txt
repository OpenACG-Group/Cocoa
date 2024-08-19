add_library(Gallium STATIC
        Platform.cc
        RuntimeBase.cc
        Runtime.cc
        TracingController.cc
        ModuleImportURL.cc
        Internals.cc
        GlobalIsolateGuard.cc
        VMIntrospect.cc
        UnixPathTools.cc
        Infrastructures.cc
        Inspector.cc
        InspectorChannel.cc
        InspectorThread.cc

        ffi/ReturnValue.cc
        ffi/Function.cc
        ffi/JSObject.cc
        ffi/ClassRegistry.cc
        ffi/Class.cc
        ffi/Module.cc
        ffi/Interface.cc
        ffi/InterfaceRegistry.cc
        ffi/Context.cc
        ffi/Enum.cc
        ffi/EnumRegistry.cc

        bindings/EventEmitter.cc

        bindings/workers/MessagePort.cc
        bindings/workers/MessagePortWrap.cc
        bindings/workers/Worker.cc
        bindings/workers/WorkerRuntime.cc
        bindings/workers/Module.cc

        bindings/typetraits/Exports.cc
        bindings/typetraits/Module.cc

        bindings/utils/Module.cc
        bindings/utils/TextCodec.cc

        bindings/fs/Module.cc
        bindings/fs/SysError.cc
        bindings/fs/FileOps.cc

        bindings/stream/Module.cc
        bindings/stream/Stream.cc

        bindings/renderer/Module.cc
        bindings/renderer/SamplingOptions.cc
        bindings/renderer/Color.cc
        bindings/renderer/Rect.cc
        bindings/renderer/ImageInfo.cc
        bindings/renderer/Pixmap.cc
        bindings/renderer/AsyncRescalable.cc
        bindings/renderer/Surface.cc
        bindings/renderer/ImageAsyncReadResult.cc
        bindings/renderer/Image.cc
        bindings/renderer/GpuDirectContext.cc
        bindings/renderer/Matrix.cc
        bindings/renderer/Canvas.cc
        bindings/renderer/Paint.cc
        bindings/renderer/Path.cc
        bindings/renderer/PathBuilder.cc
        bindings/renderer/PathEffect.cc
        bindings/renderer/Flattenable.cc
        bindings/renderer/BufferUtils.cc
        bindings/renderer/Picture.cc
        bindings/renderer/PictureRecorder.cc
        bindings/renderer/ImageFilter.cc
        bindings/renderer/ColorFilter.cc
        bindings/renderer/Blender.cc
        bindings/renderer/Shader.cc
        bindings/renderer/RuntimeEffect.cc
        bindings/renderer/FontMgr.cc
        bindings/renderer/Typeface.cc
        bindings/renderer/Font.cc
        bindings/renderer/Vertices.cc

        bindings/pixencoder/Module.cc
        bindings/pixencoder/PNGEncoder.cc
        bindings/pixencoder/JPEGEncoder.cc
        bindings/pixencoder/WebpEncoder.cc

        bindings/present/Module.cc
        bindings/present/Promisify.cc
        bindings/present/PresentThread.cc
        bindings/present/Display.cc
        bindings/present/Monitor.cc
        bindings/present/Cursor.cc
        bindings/present/Surface.cc
        bindings/present/ContentAggregator.cc
        bindings/present/Scene.cc

        bindings/multimedia/Types.cc
        bindings/multimedia/Module.cc
        bindings/multimedia/MediaInputOutput.cc
        bindings/multimedia/FormatDemuxer.cc
        bindings/multimedia/AChannelLayout.cc
        bindings/multimedia/CodecParameters.cc
        bindings/multimedia/Packet.cc
        bindings/multimedia/CodecContext.cc
        bindings/multimedia/Frame.cc
        bindings/multimedia/AudioStreamService.cc
        bindings/multimedia/AudioSinkStream.cc
        bindings/multimedia/FilterGraph.cc
        bindings/multimedia/FrameScheduler.cc
        bindings/multimedia/HWFramesContext.cc
        bindings/multimedia/HWDeviceContext.h
        bindings/multimedia/HWDeviceContext.cc
        bindings/multimedia/PixelFrameFactory.cpp
        bindings/multimedia/PixelFrameView.cpp
)
target_include_directories(Gallium PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
        Gallium
        expat
        v8
        v8_libplatform
        websockets

        ## For `paragraph` binding
        # ${COCOA_THIRDPARTY_DIR}/build/lib/libskparagraph.a
        # ${COCOA_THIRDPARTY_DIR}/build/lib/libskunicode.a
        # skparagraph
        # skunicode
        # harfbuzz

        ## For `lottie` binding
        # ${COCOA_THIRDPARTY_DIR}/build/lib/libskresources.a
        # ${COCOA_THIRDPARTY_DIR}/build/lib/libskottie.a
        # ${COCOA_THIRDPARTY_DIR}/build/lib/libsksg.a

        ## For `svg` bindings
        # ${COCOA_THIRDPARTY_DIR}/build/lib/libsvg.a

        # ${COCOA_THIRDPARTY_DIR}/build/lib/libskshaper.a
)

target_compile_definitions(
        Gallium PRIVATE
        COCOA_USE_MONOLITHIC_V8=1
        SK_SHAPER_HARFBUZZ_AVAILABLE=1
        SK_UNICODE_AVAILABLE=1
)
