QT       += core gui widgets

CONFIG += c++2a

SOURCES += \
    TrFile.cpp \
    TrProject.cpp \
    main.cpp \
    FmMain.cpp

HEADERS += \
    ../Libs/SelfMade/u_TypedFlags.h \
    FmMain.h \
    TrFile.h \
    TrProject.h

INCLUDEPATH += \
    ../Libs/SelfMade

FORMS += \
    FmMain.ui
