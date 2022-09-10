set(CIB_SCRIPT ${CMAKE_HOME_DIRECTORY}/script/collect-internal-bindings.py)
set(CIB_SUBSCRIPT ${CMAKE_HOME_DIRECTORY}/script/modulec.py)
set(CIB_MODULE_LISTS ${CMAKE_CURRENT_SOURCE_DIR}/bindings/ModuleLists.txt)
set(CIB_MODULE_XML
        bindings/core/Module.xml
        bindings/glamor/Module.xml)
set(CIB_OUTPUT_FILE
        ${CMAKE_CURRENT_BINARY_DIR}/internal_bindings.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_core.cc
        ${CMAKE_CURRENT_BINARY_DIR}/module_glamor.cc)

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
        StackAllocatorBase.h
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
        bindings/core/Property.cc
        bindings/core/Filesystem.cc
        bindings/core/Buffer.cc
        bindings/core/Process.cc
        bindings/core/Stream.cc

        bindings/glamor/Exports.h
        bindings/glamor/Exports.cc
        bindings/glamor/PromiseHelper.h
        bindings/glamor/PromiseHelper.cc
        bindings/glamor/RenderHost.cc
        bindings/glamor/DisplayWrap.cc
        bindings/glamor/SurfaceWrap.cc
        bindings/glamor/BlenderWrap.cc
        bindings/glamor/MoeTranslationToolchainWrap.cc
        bindings/glamor/CkPictureWrap.cc
        bindings/glamor/CkImageWrap.cc
        bindings/glamor/CkBitmapWrap.cc
        bindings/glamor/CkImageFilterWrap.cc
        bindings/glamor/Scene.h
        bindings/glamor/Scene.cc
        bindings/glamor/SceneBuilder.h
        bindings/glamor/SceneBuilder.cc
        bindings/glamor/CursorWrap.cc
)
target_include_directories(Gallium PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
        Gallium
        tinyxml2
        v8
        v8_libplatform
        websockets
)
