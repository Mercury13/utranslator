QT       += core gui widgets

CONFIG += c++2a

CONFIG(debug, debug|release) {
    DEFINES += AT_RANGE_CHECK
}

SOURCES += \
    ../Libs/PugiXml/pugixml.cpp \
    ../Libs/QModels/QModels.cpp \
    ../Libs/SelfMade/i_OpenSave.cpp \
    ../Libs/SelfMade/u_OpenSaveStrings.cpp \
    ../Libs/SelfMade/u_Qstrings.cpp \
    ../Libs/SelfMade/u_Strings.cpp \
    ../Libs/SelfMade/u_XmlUtils.cpp \
    FmAboutFormat.cpp \
    FmDecoder.cpp \
    FmDisambigPair.cpp \
    FmFileFormat.cpp \
    FmFind.cpp \
    History.cpp \
    TrProject/Decoders.cpp \
    TrProject/Modifiable.cpp \
    TrProject/TrDefines.cpp \
    TrProject/TrFile.cpp \
    TrProject/TrProject.cpp \
    d_Config.cpp \
    FmNew.cpp \
    main.cpp \
    FmMain.cpp

HEADERS += \
    ../Libs/PugiXml/pugiconfig.hpp \
    ../Libs/PugiXml/pugixml.hpp \
    ../Libs/QModels/QModels.h \
    ../Libs/SelfMade/i_OpenSave.h \
    ../Libs/SelfMade/u_OpenSaveStrings.h \
    ../Libs/SelfMade/u_Qstrings.h \
    ../Libs/SelfMade/u_Strings.h \
    ../Libs/SelfMade/u_TypedFlags.h \
    ../Libs/SelfMade/u_Uptr.h \
    ../Libs/SelfMade/u_XmlUtils.h \
    FmAboutFormat.h \
    FmDecoder.h \
    FmDisambigPair.h \
    FmFileFormat.h \
    FmFind.h \
    History.h \
    TrProject/Decoders.h \
    TrProject/Modifiable.h \
    TrProject/TrDefines.h \
    TrProject/TrFile.h \
    TrProject/TrProject.h \
    d_Config.h \
    FmMain.h \
    FmNew.h

INCLUDEPATH += \
    ../Libs/PugiXml \
    ../Libs/QModels \
    ../Libs/SelfMade \
    TrProject

FORMS += \
    FmAboutFormat.ui \
    FmDecoder.ui \
    FmDisambigPair.ui \
    FmFileFormat.ui \
    FmFind.ui \
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
