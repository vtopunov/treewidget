# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Add-in.
# ------------------------------------------------------

TEMPLATE = app
TARGET = carbrands
DESTDIR = ../Win32/Release
QT += core xml widgets gui
CONFIG += release
DEFINES += Q_COMPILER_INITIALIZER_LISTS WIN64 QT_DLL QT_WIDGETS_LIB QT_XML_LIB
INCLUDEPATH += ./GeneratedFiles \
    . \
    ./GeneratedFiles/Release
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/release
OBJECTS_DIR += release
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles
TRANSLATIONS += ru.ts
HEADERS += ./treeitem.h \
    ./mainwidget.h \
    ./treewidget.h \
    ./treemodel.h \
    ./sortfilterproxymodel.h \
    ./buttonsdelegate.h
SOURCES += ./buttonsdelegate.cpp \
    ./main.cpp \
    ./mainwidget.cpp \
    ./sortfilterproxymodel.cpp \
    ./treemodel.cpp \
    ./treewidget.cpp
FORMS += ./mainwidget.ui
TRANSLATIONS += ./ru.ts
RESOURCES += mainwidget.qrc