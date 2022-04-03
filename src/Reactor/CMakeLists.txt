find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_library(
        Reactor STATIC
        Reactor.h
        Reactor.cc
        ExecutableMemory.h
        ExecutableMemoryLinuxImpl.cc
        JitSession.cc
        JitSession.h
        GShaderExternals.h
        GShaderExternals.cc
        GShaderBuilder.cc
        GShaderModule.cc
)

target_include_directories(Reactor PUBLIC ${LLVM_INCLUDE_DIRS})

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(Reactor PUBLIC ${LLVM_DEFINITIONS_LIST})

# llvm_map_components_to_libnames(llvm_libs support core)
target_link_libraries(Reactor LLVM-13)
