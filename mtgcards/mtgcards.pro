#-------------------------------------------------
#
# Project created by QtCreator 2015-04-11T01:02:28
#
#-------------------------------------------------

CONFIG += c++11
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mtgcards
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    pooltablemodel.cpp \
    poolwindow.cpp \
    cardwindow.cpp \
    collectionwindow.cpp \
    deckwindow.cpp \
    deckwidget.cpp \
    optionsdialog.cpp \
    cardgraphicsview.cpp \
    richtextdelegate.cpp \
    manacostdelegate.cpp \
    manacost.cpp \
    magicsortfilterproxymodel.cpp

HEADERS  += mainwindow.h \
    pooltablemodel.h \
    poolwindow.h \
    cardwindow.h \
    collectionwindow.h \
    deckwindow.h \
    deckwidget.h \
    optionsdialog.h \
    enum.h \
    cardgraphicsview.h \
    richtext.h \
    richtextdelegate.h \
    manacostdelegate.h \
    manacost.h \
    magicsortfilterproxymodel.h \
    filter.h

FORMS    += mainwindow.ui \
    poolwindow.ui \
    cardwindow.ui \
    collectionwindow.ui \
    deckwindow.ui \
    deckwidget.ui \
    optionsdialog.ui

RESOURCES += \
    mtgcards.qrc

DISTFILES +=
