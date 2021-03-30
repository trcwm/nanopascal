cmake_minimum_required(VERSION 3.7)
project(nanopascal)

include_directories(${PROJECT_SOURCE_DIR}/common)

set(PASCALSRC
    ${PROJECT_SOURCE_DIR}/src/symtbl.c
    ${PROJECT_SOURCE_DIR}/src/parser.c
    ${PROJECT_SOURCE_DIR}/src/lexer.c
    ${PROJECT_SOURCE_DIR}/src/typestack.c
    ${PROJECT_SOURCE_DIR}/src/main.c
)

set(VMSRC 
    ${PROJECT_SOURCE_DIR}/virtualmachine/main.c
    ${PROJECT_SOURCE_DIR}/virtualmachine/vm.c
)

set(PASMSRC 
    ${PROJECT_SOURCE_DIR}/passembler/main.c
    ${PROJECT_SOURCE_DIR}/passembler/keywords.c
    ${PROJECT_SOURCE_DIR}/passembler/lex.c
    ${PROJECT_SOURCE_DIR}/passembler/symtbl.c
    ${PROJECT_SOURCE_DIR}/passembler/fixuptbl.c
    ${PROJECT_SOURCE_DIR}/passembler/parse.c
)

set(PDISASMSRC
    ${PROJECT_SOURCE_DIR}/pdisasm/main.c
    ${PROJECT_SOURCE_DIR}/passembler/keywords.c
)

add_subdirectory(vmdbgui)

add_executable(vm ${VMSRC})
add_executable(passembler ${PASMSRC})
add_executable(pdisasm ${PDISASMSRC})
add_executable(nanopascal ${PASCALSRC})
