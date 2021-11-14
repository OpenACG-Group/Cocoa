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

add_library(Koi STATIC
        KoiBase.h
        Runtime.h
        Runtime.cc
        ModuleUrl.h
        ModuleUrl.cc
        Internals.h
        Internals.cc
        BindingManager.h
        BindingManager.cc
        UUID.h

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
        lang/Base.h

        lang/CoreFDLR.h
        lang/CoreFDLR.cc
        lang/CoreBinding.h
        lang/CoreBinding.cc
        ${CMAKE_CURRENT_BINARY_DIR}/CoreBindingGen.cc

        lang/VanillaBinding.h
        lang/VanillaBinding.cc)
target_link_libraries(Koi PRIVATE v8_libplatform v8)
add_binding_exports_getter(CoreBindingGen.cc lang/CoreBinding.mds)
