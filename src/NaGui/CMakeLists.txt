add_library(NaGui STATIC
            NaGui.h
            NaGui.cc
            NaGuiRender.cc
            StatefulValue.h
            NaSharedState.h
            NaSharedState.cc)

target_compile_options(NaGui PRIVATE ${Cairo_CFLAGS})
