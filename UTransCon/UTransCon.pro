TEMPLATE = app
CONFIG += console c++2a
CONFIG -= app_bundle
CONFIG -= qt

win32-g++ {
    QMAKE_CXXFLAGS += -static-libgcc -static-libstdc++
    LIBS += -static -lpthread
}

# This is utility, range check on release is OK
DEFINES += AT_RANGE_CHECK

SOURCES += \
        ../Libs/PugiXml/pugixml.cpp \
        ../Libs/SelfMade/Strings/u_Strings.cpp \
        ../Libs/SelfMade/u_XmlUtils.cpp \
        ../UTranslator/TrProject/Decoders.cpp \
        ../UTranslator/TrProject/Modifiable.cpp \
        ../UTranslator/TrProject/TrDefines.cpp \
        ../UTranslator/TrProject/TrFile.cpp \
        ../UTranslator/TrProject/TrProject.cpp \
        main.cpp


INCLUDEPATH += \
    ../Libs/PugiXml \
    ../Libs/SelfMade \
    ../Libs/SelfMade/Strings
    ../UTranslator/TrProject

HEADERS += \
    ../Libs/PugiXml/pugiconfig.hpp \
    ../Libs/PugiXml/pugixml.hpp \
    ../Libs/SelfMade/Strings/u_Strings.h \
    ../Libs/SelfMade/u_Vector.h \
    ../Libs/SelfMade/u_XmlUtils.h \
    ../UTranslator/TrProject/Decoders.h \
    ../UTranslator/TrProject/Modifiable.h \
    ../UTranslator/TrProject/TrDefines.h \
    ../UTranslator/TrProject/TrFile.h \
    ../UTranslator/TrProject/TrProject.h
