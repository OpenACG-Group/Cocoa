add_library(NaGui STATIC
            Base.h
            Math.h
            Context.h
            Context.cc
            WindowListener.h
            WindowListener.cc
            Window.h
            Window.cc
            DrawContext.h
            Draw.h
            Draw.cc)
target_compile_options(NaGui PRIVATE ${Cairo_CFLAGS})
