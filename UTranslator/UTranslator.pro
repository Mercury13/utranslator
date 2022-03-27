QT       += core gui widgets

CONFIG += c++2a

CONFIG(debug, debug|release) {
    DEFINES += AT_RANGE_CHECK
}

SOURCES += \
    ../Libs/SelfMade/u_Qstrings.cpp \
    FmDisambigPair.cpp \
    TrProject/TrDefines.cpp \
    TrProject/TrFile.cpp \
    TrProject/TrProject.cpp \
    FmNew.cpp \
    main.cpp \
    FmMain.cpp

HEADERS += \
    ../Libs/SelfMade/u_Qstrings.h \
    ../Libs/SelfMade/u_TypedFlags.h \
    FmDisambigPair.h \
    TrProject/TrDefines.h \
    TrProject/TrFile.h \
    TrProject/TrProject.h \
    FmMain.h \
    FmNew.h

INCLUDEPATH += \
    ../Libs/SelfMade \
    TrProject

FORMS += \
    FmDisambigPair.ui \
    FmMain.ui \
    FmNew.ui
