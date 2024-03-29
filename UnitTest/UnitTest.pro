TEMPLATE = app
CONFIG += console c++2a
CONFIG -= app_bundle
CONFIG -= qt

win32-g++: {
    QMAKE_CXXFLAGS += -static-libgcc -static-libstdc++
    QMAKE_LFLAGS += -fuse-ld=lld
    LIBS += -static -lpthread
}

SOURCES += \
    ../Libs/GoogleTest/src/gtest-all.cc \
    ../Libs/GoogleTest/src/gtest_main.cc \
    ../Libs/SelfMade/Strings/u_Strings.cpp \
    ../Libs/SelfMade/Strings/u_Decoders.cpp \
    test_DecodeBr.cpp \
    test_DecodeCpp.cpp \
    test_DecodeIni.cpp \
    test_DecodeQuoted.cpp \
    test_DetectBom.cpp \
    test_EscapeCpp.cpp \
    test_EscapeText.cpp

HEADERS += \
    ../Libs/SelfMade/Strings/u_Decoders.h

INCLUDEPATH += \
    ../Libs/GoogleTest \
    ../Libs/GoogleTest/include \
    ../Libs/SelfMade \
    ../Libs/SelfMade/Strings \
    ../UTranslator/TrProject
