set(COCOA_QRESC_TARGET cocoa-qresc)

add_executable(
        ${COCOA_QRESC_TARGET}
        qresc.cc
        md5.h
        md5.cc
)

set_target_properties(${COCOA_QRESC_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_link_libraries(
        ${COCOA_QRESC_TARGET}
        tinyxml2
        ${COCOA_THIRDPARTY_DIR}/build/lib/libfmt.a)
