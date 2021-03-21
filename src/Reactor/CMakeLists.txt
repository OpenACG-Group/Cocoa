add_library(Reactor STATIC
            Reactor.h
            Runtime.cc)

target_link_libraries(Reactor PRIVATE
                      asmjit pthread rt)
