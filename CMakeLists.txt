project(nanopascal)

set(PASCALSRC
    ${PROJECT_SOURCE_DIR}/src/symtbl.c
    ${PROJECT_SOURCE_DIR}/src/parser.c
    ${PROJECT_SOURCE_DIR}/src/lexer.c
    ${PROJECT_SOURCE_DIR}/src/main.c
)

set(VMSRC 
    ${PROJECT_SOURCE_DIR}/virtualmachine/main.c
    ${PROJECT_SOURCE_DIR}/virtualmachine/vm.c
)

set(PASMSRC 
    ${PROJECT_SOURCE_DIR}/passembler/main.c
    ${PROJECT_SOURCE_DIR}/passembler/lex.c
    ${PROJECT_SOURCE_DIR}/passembler/parse.c
)

add_executable(vm ${VMSRC})
add_executable(passembler ${PASMSRC})
add_executable(nanopascal ${PASCALSRC})
