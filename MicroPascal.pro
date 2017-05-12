TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

HEADERS +=  src/tokenizer.h  \
            src/reader.h

SOURCES +=  src/main.cpp \
            src/tokenizer.cpp \
            src/reader.cpp \
#\
            #src/functiondefs.cpp
