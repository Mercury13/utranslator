TEMPLATE = app
CONFIG += console c++2a
CONFIG -= app_bundle
CONFIG -= qt

win32-g++ {
    QMAKE_CXXFLAGS += -municode -static-libgcc -static-libstdc++
    QMAKE_CXXFLAGS_RELEASE += -flto
    QMAKE_LFLAGS += -municode
    QMAKE_LFLAGS_RELEASE += -flto
    LIBS += -static -lpthread
}

# This is utility, range check on release is OK
DEFINES += AT_RANGE_CHECK

SOURCES += \
        ../Libs/PugiXml/pugixml.cpp \
        ../Libs/SelfMade/Strings/u_Decoders.cpp \
        ../Libs/SelfMade/Strings/u_Strings.cpp \
        ../Libs/SelfMade/u_Args.cpp \
        ../Libs/SelfMade/u_XmlUtils.cpp \
        ../UTranslator/TrProject/Modifiable.cpp \
        ../UTranslator/TrProject/TrDefines.cpp \
        ../UTranslator/TrProject/TrFile.cpp \
        ../UTranslator/TrProject/TrProject.cpp \
        main.cpp


INCLUDEPATH += \
    ../Libs/MagicEnum \
    ../Libs/PugiXml \
    ../Libs/SelfMade \
    ../Libs/SelfMade/Strings \
    ../Libs/Unicode \
    ../UTranslator/TrProject

HEADERS += \
    ../Libs/PugiXml/pugiconfig.hpp \
    ../Libs/PugiXml/pugixml.hpp \
    ../Libs/SelfMade/Strings/u_Decoders.h \
    ../Libs/SelfMade/Strings/u_Strings.h \
    ../Libs/SelfMade/u_Args.h \
    ../Libs/SelfMade/u_Vector.h \
    ../Libs/SelfMade/u_XmlUtils.h \
    ../Libs/Unicode/unicode.h \
    ../UTranslator/TrProject/Modifiable.h \
    ../UTranslator/TrProject/TrDefines.h \
    ../UTranslator/TrProject/TrFile.h \
    ../UTranslator/TrProject/TrProject.h
