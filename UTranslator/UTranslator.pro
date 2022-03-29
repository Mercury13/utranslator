QT       += core gui widgets

CONFIG += c++2a

CONFIG(debug, debug|release) {
    DEFINES += AT_RANGE_CHECK
}

SOURCES += \
    ../Libs/PugiXml/pugixml.cpp \
    ../Libs/SelfMade/i_OpenSave.cpp \
    ../Libs/SelfMade/u_OpenSaveStrings.cpp \
    ../Libs/SelfMade/u_Qstrings.cpp \
    ../Libs/SelfMade/u_Strings.cpp \
    FmDisambigPair.cpp \
    TrProject/Modifiable.cpp \
    TrProject/TrDefines.cpp \
    TrProject/TrFile.cpp \
    TrProject/TrProject.cpp \
    FmNew.cpp \
    main.cpp \
    FmMain.cpp

HEADERS += \
    ../Libs/PugiXml/pugiconfig.hpp \
    ../Libs/PugiXml/pugixml.hpp \
    ../Libs/SelfMade/i_OpenSave.h \
    ../Libs/SelfMade/u_OpenSaveStrings.h \
    ../Libs/SelfMade/u_Qstrings.h \
    ../Libs/SelfMade/u_Strings.h \
    ../Libs/SelfMade/u_TypedFlags.h \
    FmDisambigPair.h \
    TrProject/Modifiable.h \
    TrProject/TrDefines.h \
    TrProject/TrFile.h \
    TrProject/TrProject.h \
    FmMain.h \
    FmNew.h

INCLUDEPATH += \
    ../Libs/PugiXml \
    ../Libs/SelfMade \
    TrProject

FORMS += \
    FmDisambigPair.ui \
    FmMain.ui \
    FmNew.ui

win32 {         # and W64 too
    LIBS += -lcomdlg32

    RC_ICONS = UTranslator-win.ico
    QMAKE_TARGET_COMPANY = Mikhail Merkuryev
    QMAKE_TARGET_PRODUCT = UTranslator
    QMAKE_TARGET_DESCRIPTION = UTranslator: translation tool for Unicodia
    QMAKE_TARGET_COPYRIGHT =  Mikhail Merkuryev
}
