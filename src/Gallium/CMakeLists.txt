set(CIB_SCRIPT ${CMAKE_HOME_DIRECTORY}/script/collect-internal-bindings.py)
set(CIB_SUBSCRIPT ${CMAKE_HOME_DIRECTORY}/script/modulec.py)
set(CIB_MODULE_LISTS ${CMAKE_CURRENT_SOURCE_DIR}/bindings/ModuleLists.txt)
set(CIB_MODULE_XML
        bindings/core/Module.xml
        bindings/glamor/Module.xml
        bindings/utau/Module.xml)
set(CIB_OUTPUT_FILE
        ${CMAKE_CURRENT_BINARY_DIR}/internal_bindings.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_core.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_glamor.cc
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
        Runtime.h
        Runtime.cc
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
        InspectorClient.h
        InspectorClient.cc
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
        bindings/Base.h
        ${CIB_OUTPUT_FILE}

        bindings/core/Exports.h
        bindings/core/Exports.cc
        bindings/core/Filesystem.cc
        bindings/core/FileSyncOperations.cc
        bindings/core/Buffer.cc
        bindings/core/Process.cc
        bindings/core/Stream.cc

        bindings/glamor/Types.h
        bindings/glamor/Exports.h
        bindings/glamor/Exports.cc
        bindings/glamor/Capabilities.cc
        bindings/glamor/PromiseHelper.h
        bindings/glamor/PromiseHelper.cc
        bindings/glamor/RenderHost.cc
        bindings/glamor/DisplayWrap.cc
        bindings/glamor/SurfaceWrap.cc
        bindings/glamor/BlenderWrap.cc
        bindings/glamor/CkPictureWrap.cc
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
        bindings/glamor/CanvasKitTransferContext.h
        bindings/glamor/CanvasKitTransferContext.cc
        bindings/glamor/EffectDSLParser.h
        bindings/glamor/EffectDSLParser.cc
        bindings/glamor/EffectDSLBuilderHelperMacros.h
        bindings/glamor/TrivialSkiaExportedTypes.h
        bindings/glamor/TrivialSkiaExportedTypes.cc
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
        bindings/glamor/CkCanvasWrap.h
        bindings/glamor/CkCanvasWrap.cc
        bindings/glamor/CkTextBlobWrap.h
        bindings/glamor/CkTextBlobWrap.cc
        bindings/glamor/CkPathEffectWrap.h
        bindings/glamor/CkPathEffectWrap.cc
        bindings/glamor/CkPictureRecorder.h
        bindings/glamor/CkPictureRecorder.cc

        bindings/utau/Exports.h
        bindings/utau/Exports.cc
        bindings/utau/AudioDeviceWrap.cc
        bindings/utau/AudioSinkStreamWrap.cc
        bindings/utau/AudioBufferWrap.cc
        bindings/utau/AVFilterDAGWrap.cc
        bindings/utau/VideoBufferWrap.cc
        bindings/utau/AVStreamDecoderWrap.cc
        bindings/utau/MediaFramePresentDispatcher.cc
)
target_include_directories(Gallium PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
        Gallium
        tinyxml2
        v8
        v8_libplatform
        websockets
)
