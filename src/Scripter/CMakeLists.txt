#add_subdirectory(StaticTextCompiler)

#add_custom_command(OUTPUT StaticTextCompiled.cc
#                   COMMAND ./StaticTextCompiler/compiler ${CMAKE_CURRENT_SOURCE_DIR}
#                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#                   DEPENDS compiler)

add_library(Scripter STATIC
            ScripterBase.h
            ArrayBufferAllocator.h
            Runtime.h
            Runtime.cc
        Ops.h
        Ops.cc
        OpParameterInfo.cc)
target_link_libraries(Scripter PRIVATE v8_libplatform v8)
