QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    detectormain.cpp \
    utils/rectools.cpp

HEADERS += \
    detectormain.h \
    utils/rectools.h

FORMS += \
    detectormain.ui

INCLUDEPATH += $$PWD/utils/aubio/include
INCLUDEPATH += $$PWD/utils/portaudio/include
INCLUDEPATH += $$PWD/utils/midifile/include
INCLUDEPATH += "D:/cppLibraries/boost_1_75_0"

LIBS += -L$$PWD/utils/aubio/lib
LIBS += -L$$PWD/utils/portaudio/lib/.libs
LIBS += -L$$PWD/utils/midifile/lib

LIBS += -laubio
LIBS += -lportaudio
LIBS += -lmidifile

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
