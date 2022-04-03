#add_subdirectory(StaticTextCompiler)

#add_custom_command(OUTPUT StaticTextCompiled.cc
#                   COMMAND ./StaticTextCompiler/compiler ${CMAKE_CURRENT_SOURCE_DIR}
#                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#                   DEPENDS compiler)

macro(add_binding_exports_getter generated mdspath)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${generated}
            COMMAND ${CMAKE_HOME_DIRECTORY}/script/generate-binding-exports-getter.py
                    ${CMAKE_CURRENT_SOURCE_DIR}/${mdspath} > ${CMAKE_CURRENT_BINARY_DIR}/${generated}
            DEPENDS ${mdspath}
    )
endmacro()

add_library(Gallium STATIC
        Gallium.h
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

        bindings/core/Exports.h
        bindings/core/CoreBinding.cc
        bindings/core/Property.cc
        bindings/core/FdRandomize.h
        bindings/core/FdRandomize.cc
        bindings/core/Filesystem.cc
        bindings/core/Buffer.cc
        bindings/core/Process.cc
        bindings/core/Stream.cc
        ${CMAKE_CURRENT_BINARY_DIR}/CoreBindingGen.cc

        bindings/cobalt/Exports.h
        bindings/cobalt/Exports.cc
        bindings/cobalt/PromiseHelper.h
        bindings/cobalt/PromiseHelper.cc
        bindings/cobalt/RenderHost.cc
        bindings/cobalt/DisplayWrap.cc
        bindings/cobalt/SurfaceWrap.cc
        ${CMAKE_CURRENT_BINARY_DIR}/CobaltBindingGen.cc

        bindings/reactor/Exports.h
        bindings/reactor/Exports.cc
        bindings/reactor/GShaderBuilderWrap.cc
        bindings/reactor/GShaderModuleWrap.cc
        ${CMAKE_CURRENT_BINARY_DIR}/ReactorBindingGen.cc
)

target_link_libraries(
        Gallium
        tinyxml2
        ${COCOA_THIRDPARTY_DIR}/v8/out/release/obj/libv8_monolith.a
)

add_binding_exports_getter(CoreBindingGen.cc bindings/core/exports.mds)
add_binding_exports_getter(CobaltBindingGen.cc bindings/cobalt/exports.mds)
add_binding_exports_getter(ReactorBindingGen.cc bindings/reactor/exports.mds)
