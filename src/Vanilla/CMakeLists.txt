set(target_name Vanilla)

find_package(Vulkan REQUIRED)

add_library(${target_name} STATIC
            Base.h
            Base.cc
            Typetraits.h
            Context.h
            Context.cc
            Display.h
            Display.cc
            DrawContext.h
            DrawContext.cc
            VkDrawContext.h
            VkDrawContext.cc
            Window.h
            Window.cc
            KeySymbols.h
            KeySymbols.cc
            KeyboardProxy.h
            KeyboardProxy.cc
            Xcb/EventHandlerMacros.h
            Xcb/XcbDisplay.h
            Xcb/XcbDisplay.cc
            Xcb/XcbDisplayXInput.cc
            Xcb/XcbAtoms.h
            Xcb/XcbAtoms.cc
            Xcb/XcbWindow.h
            Xcb/XcbWindow.cc
            Xcb/XcbKeyboard.h
            Xcb/XcbKeyboard.cc
            Xcb/XcbEventQueue.h
            Xcb/XcbEventQueue.cc
            Xcb/XcbRasterDrawContext.h
            Xcb/XcbRasterDrawContext.cc
            Audio/PulseConnection.h
            Audio/PulseConnection.cc
            Audio/AuBuffer.h
            Codec/AVDecoder.h
            Codec/AVDecoderPrivate.h
            Codec/AVDecoderDemuxer.cc
            Codec/AVDecoderDecompressor.cc
            Codec/AVFrame.h
            Codec/AVFramePrivate.h
            Codec/AVVideoFrame.cc
            Codec/AVAudioFrame.cc
            Codec/VideoHWDeviceContext.h
            Codec/VideoHWDeviceContext.cc)

set(XCB_BACKEND_LIBS
        xcb
        xcb-image
        xcb-util
        xcb-xkb
        xcb-xinput
        xkbcommon
        xkbcommon-x11)

set(CODEC_LIBS
        avformat
        avutil
        avcodec
        swscale
        yuv)

set(RENDERER_LIBS
        skia
        Vulkan::Vulkan)

set(AUDIO_LIBS pulse)

target_compile_definitions(${target_name} PRIVATE SK_VULKAN ${DBus1_DEFINITIONS})
target_link_libraries(${target_name}
        PkgConfig::SIGC
        ${RENDERER_LIBS}
        ${XCB_BACKEND_LIBS}
        ${CODEC_LIBS}
        ${AUDIO_LIBS})
