set(TESTS_EXECUTABLE cocoa-tests)

add_executable(
        ${TESTS_EXECUTABLE}

        Tests.cc
        ScalableWriteBufferTest.cc
        ScalableWriteBufferTest.h
)

target_link_libraries(
        ${TESTS_EXECUTABLE}
        Core
        ${LINK_STATIC_LIBUV}
        ${LINK_STATIC_FMT}
        ${LINK_STATIC_LIBSQUASH}
)
