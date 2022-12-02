add_library(native-libtess2 SHARED
        src/tesselator/bucketalloc.h
        src/tesselator/bucketalloc.c
        src/tesselator/dict.h
        src/tesselator/dict.c
        src/tesselator/geom.h
        src/tesselator/geom.c
        src/tesselator/mesh.h
        src/tesselator/mesh.c
        src/tesselator/priorityq.h
        src/tesselator/priorityq.c
        src/tesselator/sweep.h
        src/tesselator/sweep.c
        src/tesselator/tess.h
        src/tesselator/tess.c
        src/tesselator.h

        src/Exports.h
        src/DynamicLoaderHook.cc
        src/TessellatorWrap.cc

        src/native-libtess2.h
        src/native-libtess2.cc
)

target_link_libraries(native-libtess2 PRIVATE ${LINK_STATIC_FMT})
