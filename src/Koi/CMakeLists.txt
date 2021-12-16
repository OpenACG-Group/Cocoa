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

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/InternalsSfsBytes.cc
        COMMAND ${CMAKE_HOME_DIRECTORY}/script/compress-internal-scripts.py
                ${CMAKE_CURRENT_SOURCE_DIR}/internals > ${CMAKE_CURRENT_BINARY_DIR}/InternalsSfsBytes.cc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/internals/**
)

add_library(Koi STATIC
        KoiBase.h
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
        bindings/core/Timer.cc
        bindings/core/FdRandomize.h
        bindings/core/FdRandomize.cc
        bindings/core/Filesystem.cc
        bindings/core/Buffer.cc
        bindings/core/Process.cc
        ${CMAKE_CURRENT_BINARY_DIR}/CoreBindingGen.cc
        ${CMAKE_CURRENT_BINARY_DIR}/InternalsSfsBytes.cc)

target_link_libraries(Koi PRIVATE v8_libplatform v8 tinyxml2)

add_binding_exports_getter(CoreBindingGen.cc bindings/core/exports.mds)
