TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

HEADERS +=  src/tokenizer.h  \
            src/reader.h \
            src/parser.h \
            src/ast.h

SOURCES +=  src/main.cpp \
            src/tokenizer.cpp \
            src/reader.cpp \
            src/parser.cpp \
            src/ast.cpp
#\
            #src/functiondefs.cpp
