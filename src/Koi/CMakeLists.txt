#add_subdirectory(StaticTextCompiler)

#add_custom_command(OUTPUT StaticTextCompiled.cc
#                   COMMAND ./StaticTextCompiler/compiler ${CMAKE_CURRENT_SOURCE_DIR}
#                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#                   DEPENDS compiler)

add_library(Koi STATIC
            KoiBase.h
            ArrayBufferAllocator.h
            Runtime.h
            Runtime.cc
            Ops.h
            Ops.cc
            OpBaseImpl.cc
            OpTimerImpl.cc
            OpRenderingImpl.cc
            OpParameterInfo.cc
            ModuleUrl.h
            ModuleUrl.cc
            Internals.h
            Internals.cc
            ResourceDescriptorPool.h
            ResourceDescriptorPool.cc
            ResourceObject.h
            ResourceObject.cc
        GpTimer.h
        GpTimer.cc
        GpObjectWrapper.h)
target_link_libraries(Koi PRIVATE v8_libplatform v8)
