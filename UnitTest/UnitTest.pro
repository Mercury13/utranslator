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
    ../Libs/SelfMade/u_Strings.cpp \
    ../UTranslator/TrProject/Decoders.cpp \
    test_DecodeBr.cpp \
    test_DecodeCpp.cpp \
    test_EscapeCpp.cpp

HEADERS += \
    ../UTranslator/TrProject/Decoders.h

INCLUDEPATH += \
    ../Libs/GoogleTest \
    ../Libs/GoogleTest/include \
    ../Libs/SelfMade \
    ../UTranslator/TrProject
