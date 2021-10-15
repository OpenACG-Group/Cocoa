#add_subdirectory(StaticTextCompiler)

#add_custom_command(OUTPUT StaticTextCompiled.cc
#                   COMMAND ./StaticTextCompiler/compiler ${CMAKE_CURRENT_SOURCE_DIR}
#                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#                   DEPENDS compiler)

add_library(Koi STATIC
        KoiBase.h
        Runtime.h
        Runtime.cc
        ModuleUrl.h
        ModuleUrl.cc
        Internals.h
        Internals.cc
        BindingsLoader.h
        BindingsLoader.cc

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
        lang/Base.cc
        lang/CoreBinding.h
        lang/CoreBinding.cc
        lang/VanillaBinding.h
        lang/VanillaBinding.cc)
target_link_libraries(Koi PRIVATE v8_libplatform v8)
