#-------------------------------------------------
#
# Project created by QtCreator 2017-09-29T20:05:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CAN_test
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
    mainwindow.cpp \
    sample.cpp

HEADERS += \
    mainwindow.h \
    win32/can_driver.h \
    win32/ControlCAN.h \
    sample.h \
    sample.h

FORMS += \
        mainwindow.ui \
    sample.ui



win32:CONFIG(release, debug|release): LIBS += -L$$PWD/win32/ -lControlCAN
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/win32/ -lControlCAN
else:unix: LIBS += -L$$PWD/win32/ -lControlCAN

INCLUDEPATH += $$PWD/win32
DEPENDPATH += $$PWD/win32

RESOURCES += \
    can_test.qrc
RC_FILE +=ico.rc
