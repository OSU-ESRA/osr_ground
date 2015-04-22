#-------------------------------------------------
#
# Project created by QtCreator 2013-09-24T17:03:18
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qfi_example
TEMPLATE = app

#-------------------------------------------------

win32: DEFINES += WIN32 _WINDOWS _USE_MATH_DEFINES

win32:CONFIG(release, debug|release):    DEFINES += NDEBUG
else:win32:CONFIG(debug, debug|release): DEFINES += _DEBUG

#-------------------------------------------------

INCLUDEPATH += ./ ./qfi

#-------------------------------------------------

HEADERS += \
    qfi/LayoutSquare.h \
    qfi/MainWindow.h \
    qfi/WidgetPFD.h \
    qfi_PFD.h

SOURCES += \
    qfi/LayoutSquare.cpp \
    qfi/main.cpp \
    qfi/MainWindow.cpp \
    qfi/WidgetPFD.cpp \
    qfi_PFD.cpp

FORMS += \
    qfi/MainWindow.ui \
    qfi/WidgetPFD.ui

RESOURCES += \
    qfi.qrc
