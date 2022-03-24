QT       += core gui widgets

CONFIG += c++2a

SOURCES += \
    ../Libs/SelfMade/u_Qstrings.cpp \
    TrProject/TrDefines.cpp \
    TrProject/TrFile.cpp \
    TrProject/TrProject.cpp \
    FmNew.cpp \
    main.cpp \
    FmMain.cpp

HEADERS += \
    ../Libs/SelfMade/u_Qstrings.h \
    ../Libs/SelfMade/u_TypedFlags.h \
    TrProject/TrDefines.h \
    TrProject/TrFile.h \
    TrProject/TrProject.h \
    FmMain.h \
    FmNew.h

INCLUDEPATH += \
    ../Libs/SelfMade \
    TrProject

FORMS += \
    FmMain.ui \
    FmNew.ui
