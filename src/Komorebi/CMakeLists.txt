#add_custom_command(OUTPUT StaticTextCompiled.cc
#                   COMMAND ./StaticTextCompiler/compiler ${CMAKE_CURRENT_SOURCE_DIR}
#                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#                   DEPENDS compiler)

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/KRSLScanner.cc
                          ${CMAKE_CURRENT_SOURCE_DIR}/KRSLParser.hh
                          ${CMAKE_CURRENT_SOURCE_DIR}/KRSLParser.cc
                          ${CMAKE_CURRENT_SOURCE_DIR}/location.hh
                   COMMAND flex -o KRSLScanner.cc KRSLScanner.l
                           && bison -o KRSLParser.cc KRSLParser.y
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(komorebi_src
        Namespace.h
        KRSLParserDriver.h
        KRSLParserDriver.cc
        KRSLScanner.h
        KRSLScanner.cc
        KRSLParser.cc
        KRSLParser.hh
        ASTBaseNode.h
        ASTBaseNode.cc
        ASTCodeBlockNode.h
        ASTValue.h
        location.hh
        ShaderModule.h
        ShaderModule.cc
        ShaderProgram.h
        ShaderProgram.cc)

add_library(Komorebi STATIC ${komorebi_src})
#target_compile_options(Komorebi PRIVATE "-fno-rtti")
target_include_directories(Komorebi PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Komorebi PRIVATE
        LLVMOrcJIT
        LLVMExecutionEngine
        LLVMCore
        LLVMSupport
        LLVMX86CodeGen
        LLVMX86AsmParser
        LLVMX86Desc
        LLVMX86Info)
