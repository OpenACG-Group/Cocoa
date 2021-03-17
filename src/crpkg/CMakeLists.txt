set(crpkg_sources
        crpkg.h
        proto.h
        Deserializer.h
        Deserializer.cc
        INodeTree.h
        INodeTree.cc
        Serializer.h
        Serializer.cc
        Compression.h
        Compression.cc)

add_library(crpkg STATIC ${crpkg_sources})
target_link_libraries(
        crpkg
        PRIVATE
        libzstd_static)

add_executable(
        crpkg-tool
        crpkg-tool.cc
        ../Core/Exception.cc
        ../Core/PropertyTree.cc)

target_link_libraries(
        crpkg-tool
        PRIVATE
        crpkg
        unwind
        dl
        Poco::Foundation)
