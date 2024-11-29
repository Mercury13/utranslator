QT       += core gui widgets svgwidgets
CONFIG += c++20 c++2a

CONFIG(debug, debug|release) {
    DEFINES += AT_RANGE_CHECK
}
DEFINES += QT_STRINGS

SOURCES += \
    ../Libs/PugiXml/pugixml.cpp \
    ../Libs/QModels/QModels.cpp \
    ../Libs/Qt/DblClickLabel.cpp \
    ../Libs/Qt/DblClickRadio.cpp \
    ../Libs/Qt/ElidedLabel.cpp \
    ../Libs/SelfMade/L10n/LocFmt.cpp \
    ../Libs/SelfMade/Qt/QtMultiRadio.cpp \
    ../Libs/SelfMade/i_OpenSave.cpp \
    ../Libs/SelfMade/u_OpenSaveStrings.cpp \
    ../Libs/SelfMade/u_XmlUtils.cpp \
    ../Libs/SelfMade/Strings/u_Decoders.cpp \
    ../Libs/SelfMade/Strings/u_Qstrings.cpp \
    ../Libs/SelfMade/Strings/u_Strings.cpp \
    FmAboutFormat.cpp \
    FmDisambigPair.cpp \
    FmFileFormat.cpp \
    FmFind.cpp \
    FmMessage.cpp \
    FmProjectSettings.cpp \
    History.cpp \
    Main/DiffBrowser.cpp \
    Main/FmMain.cpp \
    Main/PrjTreeModel.cpp \
    Main/QtDiff.cpp \
    Main/WiFind.cpp \
    QtIconLib.cpp \
    Tools/FmDecoder.cpp \
    Tools/FmTranslateWithOriginal.cpp \
    Tools/FmSwitchOriginalAndTranslation.cpp \
    Tools/FmExtractOriginal.cpp \
    TrProject/Modifiable.cpp \
    TrProject/TrBugs.cpp \
    TrProject/TrDefines.cpp \
    TrProject/TrFile.cpp \
    TrProject/TrFinder.cpp \
    TrProject/TrProject.cpp \
    TrProject/TrUtils.cpp \
    TrProject/TrWrappers.cpp \
    d_Config.cpp \
    FmNew.cpp \
    main.cpp

HEADERS += \
    ../Libs/MagicEnum/magic_enum.hpp \
    ../Libs/PugiXml/pugiconfig.hpp \
    ../Libs/PugiXml/pugixml.hpp \
    ../Libs/QModels/QModels.h \
    ../Libs/Qt/DblClickLabel.h \
    ../Libs/Qt/DblClickRadio.h \
    ../Libs/Qt/ElidedLabel.h \
    ../Libs/SelfMade/Cpp03.h \
    ../Libs/SelfMade/L10n/LocFmt.h \
    ../Libs/SelfMade/Mojibake/mojibake.h \
    ../Libs/SelfMade/Qt/QtConsts.h \
    ../Libs/SelfMade/Qt/QtMultiRadio.h \
    ../Libs/SelfMade/i_OpenSave.h \
    ../Libs/SelfMade/u_Array.h \
    ../Libs/SelfMade/u_EcArray.h \
    ../Libs/SelfMade/u_OpenSaveStrings.h \
    ../Libs/SelfMade/u_TypedFlags.h \
    ../Libs/SelfMade/u_Uptr.h \
    ../Libs/SelfMade/u_XmlUtils.h \
    ../Libs/SelfMade/Strings/u_Decoders.h \
    ../Libs/SelfMade/Strings/u_Qstrings.h \
    ../Libs/SelfMade/Strings/u_Strings.h \
    FmAboutFormat.h \
    FmDisambigPair.h \
    FmFileFormat.h \
    FmFind.h \
    FmMessage.h \
    FmProjectSettings.h \
    History.h \
    Main/DiffBrowser.h \
    Main/FmMain.h \
    Main/PrjTreeModel.h \
    Main/QtDiff.h \
    Main/WiFind.h \
    QtIconLib.h \
    Tools/FmDecoder.h \
    Tools/FmExtractOriginal.h \
    Tools/FmSwitchOriginalAndTranslation.h \
    Tools/FmTranslateWithOriginal.h \
    TrProject/Modifiable.h \
    TrProject/TrBugs.h \
    TrProject/TrDefines.h \
    TrProject/TrFile.h \
    TrProject/TrFinder.h \
    TrProject/TrProject.h \
    TrProject/TrUtils.h \
    TrProject/TrWrappers.h \
    d_Config.h \
    FmNew.h \
    d_Strings.h

INCLUDEPATH += \
    ../Libs \
    ../Libs/MagicEnum \
    ../Libs/PugiXml \
    ../Libs/Qt \
    ../Libs/QModels \
    ../Libs/SelfMade \
    ../Libs/SelfMade/L10n \
    ../Libs/SelfMade/Mojibake \
    ../Libs/SelfMade/Qt \
    ../Libs/SelfMade/Strings \
    Main \
    TrProject

FORMS += \
    Main/FmMain.ui \
    Main/WiFind.ui \
    Tools/FmDecoder.ui \
    Tools/FmExtractOriginal.ui \
    Tools/FmSwitchOriginalAndTranslation.ui \
    Tools/FmTranslateWithOriginal.ui \
    FmAboutFormat.ui \
    FmDisambigPair.ui \
    FmFileFormat.ui \
    FmFind.ui \
    FmNew.ui \
    FmProjectSettings.ui

win32 {         # and W64 too
    LIBS += -lcomdlg32

    RC_ICONS = UTranslator-win.ico
    QMAKE_TARGET_COMPANY = Mikhail Merkuryev
    QMAKE_TARGET_PRODUCT = UTranslator
    QMAKE_TARGET_DESCRIPTION = UTranslator: translation tool for Unicodia
    QMAKE_TARGET_COPYRIGHT =  Mikhail Merkuryev
}

win32-g++ {
    # To simplify debugging, we statically link these libraries
    QMAKE_CXXFLAGS_DEBUG += -static-libgcc -static-libstdc++
    CONFIG(debug, debug|release) {
        LIBS += -static -lpthread
    }
    # Qt — system headers
    QMAKE_CXXFLAGS += -isystem $$[QT_INSTALL_HEADERS]
}

RESOURCES += \
    Resources/req.qrc

VERSION_FILE = ../VERSION
VERSION = $$cat($${VERSION_FILE})
{ # To trigger QMAKE
    include($${VERSION_FILE})
}
