#-------------------------------------------------
#
# Project created by QtCreator 2015-04-11T01:02:28
#
#-------------------------------------------------

CONFIG += c++11
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MTGCards
ICON = resources/icons/mtgcards.icns
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
    manacost.cpp \
    magicsortfilterproxymodel.cpp \
    aboutdialog.cpp \
    filtereditordialog.cpp \
    filtermodel.cpp \
    magiccolumntype.cpp \
    magiclayouttype.cpp \
    magiccarddata.cpp \
    magicitemdelegate.cpp \
    filter.cpp \
    magicconvert.cpp \
    filters/regexfilterfunction.cpp \
    filters/datefilterfunction.cpp \
    settings.cpp \
    collectiontablemodel.cpp \
    magiccollection.cpp

HEADERS  += mainwindow.h \
    pooltablemodel.h \
    poolwindow.h \
    cardwindow.h \
    collectionwindow.h \
    deckwindow.h \
    deckwidget.h \
    optionsdialog.h \
	cardgraphicsview.h \
    manacost.h \
    magicsortfilterproxymodel.h \
    filter.h \
    aboutdialog.h \
    filtereditordialog.h \
    filtermodel.h \
    magiccolumntype.h \
    magiclayouttype.h \
    magiccarddata.h \
    magicitemdelegate.h \
    magicconvert.h \
    filters/regexfilterfunction.h \
    filters/datefilterfunction.h \
    settings.h \
    collectiontablemodel.h \
    magiccollection.h

FORMS    += mainwindow.ui \
    poolwindow.ui \
    cardwindow.ui \
    collectionwindow.ui \
    deckwindow.ui \
    deckwidget.ui \
    optionsdialog.ui \
    aboutdialog.ui \
    filtereditordialog.ui

RESOURCES += \
    mtgcards.qrc
