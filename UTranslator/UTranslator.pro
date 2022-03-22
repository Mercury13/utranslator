QT       += core gui widgets

CONFIG += c++2a

SOURCES += \
    FmNew.cpp \
    TrDefines.cpp \
    TrFile.cpp \
    TrProject.cpp \
    main.cpp \
    FmMain.cpp

HEADERS += \
    ../Libs/SelfMade/u_TypedFlags.h \
    FmMain.h \
    FmNew.h \
    TrDefines.h \
    TrFile.h \
    TrProject.h

INCLUDEPATH += \
    ../Libs/SelfMade

FORMS += \
    FmMain.ui \
    FmNew.ui
