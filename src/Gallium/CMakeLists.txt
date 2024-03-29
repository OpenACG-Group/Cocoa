set(CIB_SCRIPT ${CMAKE_HOME_DIRECTORY}/script/collect-internal-bindings.py)
set(CIB_SUBSCRIPT ${CMAKE_HOME_DIRECTORY}/script/modulec.py)
set(CIB_MODULE_LISTS ${CMAKE_CURRENT_SOURCE_DIR}/bindings/ModuleLists.txt)
set(CIB_MODULE_XML
        bindings/workers/Module.xml
        bindings/core/Module.xml
        bindings/typetraits/Module.xml
        bindings/glamor/Module.xml
        bindings/pixencoder/Module.xml
        bindings/paragraph/Module.xml
        bindings/resources/Module.xml
        bindings/lottie/Module.xml
        bindings/svg/Module.xml
        bindings/utau/Module.xml)

set(CIB_OUTPUT_FILE
        ${CMAKE_CURRENT_BINARY_DIR}/internal_bindings.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_workers.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_core.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_typetraits.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_glamor.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_pixencoder.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_paragraph.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_resources.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_lottie.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_svg.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_utau.cc)

add_custom_command(
        OUTPUT ${CIB_OUTPUT_FILE}
        DEPENDS ${CIB_MODULE_LISTS} ${CIB_MODULE_XML} ${CIB_SCRIPT} ${CIB_SUBSCRIPT}
        COMMAND ${CIB_SCRIPT} ${CIB_MODULE_LISTS} ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(Gallium STATIC
        Gallium.h
        Platform.h
        Platform.cc
        RuntimeBase.h
        RuntimeBase.cc
        Runtime.h
        Runtime.cc
        TracingController.h
        TracingController.cc
        ModuleImportURL.h
        ModuleImportURL.cc
        Internals.h
        Internals.cc
        BindingManager.h
        BindingManager.cc
        GlobalIsolateGuard.h
        GlobalIsolateGuard.cc
        VMIntrospect.h
        VMIntrospect.cc
        UnixPathTools.h
        UnixPathTools.cc
        Infrastructures.h
        Infrastructures.cc
        Inspector.h
        Inspector.cc
        InspectorChannel.h
        InspectorChannel.cc
        InspectorThread.h
        InspectorThread.cc
        binder/PtrTraits.h
        binder/Utility.h
        binder/Convert.h
        binder/Convert.cc
        binder/CallFromV8.h
        binder/ThrowExcept.h
        binder/ThrowExcept.cc
        binder/Function.h
        binder/Function.cc
        binder/Property.h
        binder/Factory.h
        binder/Class.h
        binder/Class.cc
        binder/Module.h
        binder/CallV8.h
        binder/TypeTraits.h
        bindings/Base.h
        bindings/ExportableObjectBase.h
        bindings/EventEmitter.h
        bindings/EventEmitter.cc
        ${CIB_OUTPUT_FILE}

        bindings/workers/Exports.h
        bindings/workers/MessagePort.h
        bindings/workers/MessagePort.cc
        bindings/workers/MessagePortWrap.cc
        bindings/workers/Worker.cc
        bindings/workers/WorkerRuntime.h
        bindings/workers/WorkerRuntime.cc

        bindings/core/Exports.h
        bindings/core/Exports.cc
        bindings/core/Filesystem.cc
        bindings/core/FileSyncOperations.cc
        bindings/core/Buffer.cc
        bindings/core/Process.cc
        bindings/core/Stream.cc
        bindings/core/TTYStreamWrap.cc
        bindings/core/CallbackScopedBuffer.cc

        bindings/typetraits/Exports.h
        bindings/typetraits/Exports.cc

        bindings/glamor/Types.h
        bindings/glamor/Exports.h
        bindings/glamor/Exports.cc
        bindings/glamor/SkiaMemoryTrace.cc
        bindings/glamor/Capabilities.cc
        bindings/glamor/PromiseHelper.h
        bindings/glamor/PromiseHelper.cc
        bindings/glamor/PresentThreadWrap.cc
        bindings/glamor/DisplayWrap.cc
        bindings/glamor/SurfaceWrap.cc
        bindings/glamor/ContentAggregatorWrap.cc
        bindings/glamor/CkPictureWrap.cc
        bindings/glamor/CkImageWrap.h
        bindings/glamor/CkImageWrap.cc
        bindings/glamor/CkBitmapWrap.cc
        bindings/glamor/CkImageFilterWrap.cc
        bindings/glamor/CkColorFilterWrap.cc
        bindings/glamor/CkShaderWrap.cc
        bindings/glamor/CkBlenderWrap.cc
        bindings/glamor/Scene.h
        bindings/glamor/Scene.cc
        bindings/glamor/SceneBuilder.h
        bindings/glamor/SceneBuilder.cc
        bindings/glamor/CursorWrap.cc
        bindings/glamor/EffectDSLParser.h
        bindings/glamor/EffectDSLParser.cc
        bindings/glamor/EffectDSLBuilderHelperMacros.h
        bindings/glamor/TrivialInterface.h
        bindings/glamor/TrivialInterface.cc
        bindings/glamor/GProfilerWrap.cc
        bindings/glamor/CkMatrixWrap.h
        bindings/glamor/CkMatrixWrap.cc
        bindings/glamor/CkSurfaceWrap.h
        bindings/glamor/CkSurfaceWrap.cc
        bindings/glamor/CkPaintWrap.h
        bindings/glamor/CkPaintWrap.cc
        bindings/glamor/CkPathWrap.h
        bindings/glamor/CkPathWrap.cc
        bindings/glamor/CkTypefaceWrap.h
        bindings/glamor/CkTypefaceWrap.cc
        bindings/glamor/CkFontWrap.h
        bindings/glamor/CkFontWrap.cc
        bindings/glamor/CkFontMgrWrap.h
        bindings/glamor/CkFontMgrWrap.cc
        bindings/glamor/CkCanvasWrap.h
        bindings/glamor/CkCanvasWrap.cc
        bindings/glamor/CkTextBlobWrap.h
        bindings/glamor/CkTextBlobWrap.cc
        bindings/glamor/CkPathEffectWrap.h
        bindings/glamor/CkPathEffectWrap.cc
        bindings/glamor/CkPictureRecorder.h
        bindings/glamor/CkPictureRecorder.cc
        bindings/glamor/CkRuntimeEffectWrap.h
        bindings/glamor/CkRuntimeEffectWrap.cc
        bindings/glamor/CkVerticesWrap.h
        bindings/glamor/CkVerticesWrap.cc
        bindings/glamor/CkPathMeasureWrap.h
        bindings/glamor/CkPathMeasureWrap.cc
        bindings/glamor/ConcurrentVertexProcessor.h
        bindings/glamor/ConcurrentVertexProcessor.cc
        bindings/glamor/GpuDirectContext.h
        bindings/glamor/GpuDirectContext.cc
        bindings/glamor/GpuExportedFd.h
        bindings/glamor/GpuExportedFd.cc
        bindings/glamor/CkPixmapWrap.h
        bindings/glamor/CkPixmapWrap.cc
        bindings/glamor/CkSurfaceContentTracker.h
        bindings/glamor/CkSurfaceContentTracker.cc

        bindings/pixencoder/Exports.h
        bindings/pixencoder/Exports.cc
        bindings/pixencoder/PNGEncoder.cc
        bindings/pixencoder/JPEGEncoder.cc
        bindings/pixencoder/WebpEncoder.cc

        bindings/paragraph/Exports.h
        bindings/paragraph/Exports.cc
        bindings/paragraph/TrivialInterfaces.cc
        bindings/paragraph/TextStyleWrap.cc
        bindings/paragraph/ParagraphStyleWrap.cc
        bindings/paragraph/ParagraphBuilderWrap.cc
        bindings/paragraph/ParagraphWrap.cc

        bindings/resources/Exports.h
        bindings/resources/Exports.cc
        bindings/resources/ResourceProvider.cc
        bindings/resources/CRPKGResources.cc

        bindings/lottie/Exports.h
        bindings/lottie/Exports.cc
        bindings/lottie/AnimationBuilderWrap.cc
        bindings/lottie/AnimationWrap.cc

        bindings/svg/Exports.h
        bindings/svg/Exports.cc
        bindings/svg/TrivialInterfaces.h
        bindings/svg/TrivialInterfaces.cc
        bindings/svg/SVGCanvasWrap.cc
        bindings/svg/SVGDOMLoaderWrap.cc
        bindings/svg/SVGDOMWrap.cc
        bindings/svg/SVGLengthContextWrap.cc

        bindings/utau/Exports.h
        bindings/utau/Exports.cc
        bindings/utau/HWFramesContextRef.cc
        bindings/utau/AudioDeviceWrap.cc
        bindings/utau/AudioSinkStreamWrap.cc
        bindings/utau/AudioBufferWrap.cc
        bindings/utau/AVFilterDAGWrap.cc
        bindings/utau/VideoBufferWrap.cc
        bindings/utau/AVStreamDecoderWrap.cc
        bindings/utau/MediaFramePresentDispatcher.h
        bindings/utau/MediaFramePresentDispatcher.cc
)
target_include_directories(Gallium PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
        Gallium
        expat
        v8
        v8_libplatform
        websockets

        ## For `paragraph` binding
        ${COCOA_THIRDPARTY_DIR}/build/lib/libskparagraph.a
        ${COCOA_THIRDPARTY_DIR}/build/lib/libskunicode.a
        harfbuzz

        ## For `lottie` binding
        ${COCOA_THIRDPARTY_DIR}/build/lib/libskresources.a
        ${COCOA_THIRDPARTY_DIR}/build/lib/libskottie.a
        ${COCOA_THIRDPARTY_DIR}/build/lib/libsksg.a

        ## For `svg` bindings
        ${COCOA_THIRDPARTY_DIR}/build/lib/libsvg.a

        ${COCOA_THIRDPARTY_DIR}/build/lib/libskshaper.a
)

target_compile_definitions(
        Gallium PRIVATE
        COCOA_USE_MONOLITHIC_V8=1
        SK_SHAPER_HARFBUZZ_AVAILABLE=1
        SK_UNICODE_AVAILABLE=1
)
