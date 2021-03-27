#-------------------------------------------------
#
# Project created by QtCreator 2015-04-11T01:02:28
#
#-------------------------------------------------

CONFIG += c++11
QT += core gui widgets network sql

macx {
	QT += svg
}

TARGET = MTGCards
ICON = resources/icons/mtgcards.icns
RC_ICONS = resources/icons/mtgcards.ico
TEMPLATE = app

VERSION = $$cat(VERSION.txt)
DEFINES += "MTGCARDS_VERSION=\"\\\"$$VERSION\\\"\""

SOURCES += main.cpp\
    carddock.cpp \
    collectiondock.cpp \
    commonactions.cpp \
    imagetablemodel.cpp \
    imagetableview.cpp \
        mainwindow.cpp \
    pooldock.cpp \
    pooltablemodel.cpp \
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
    prices.cpp \
    settings.cpp \
    collectiontablemodel.cpp \
    magiccollection.cpp \
    shortcuttype.cpp \
    deck.cpp \
    decktablemodel.cpp \
    deckmanager.cpp \
    statusbar.cpp \
    tableview.cpp \
    util.cpp \
    deckstatisticsdialog.cpp \
    manacurvelinewidget.cpp \
    splashscreen.cpp \
    tags.cpp \
	filters/comparetype.cpp \
	filters/containsfilterfunction.cpp \
	filters/regexfilterfunction.cpp \
	filters/numberfilterfunction.cpp \
	filters/deckfilterfunction.cpp \
	filters/datefilterfunction.cpp \
	filters/timefilterfunction.cpp \
    filters/tagfilterfunction.cpp

HEADERS  += mainwindow.h \
    carddock.h \
    collectiondock.h \
    commonactions.h \
    imagetablemodel.h \
    imagetableview.h \
    json.hpp \
    pooldock.h \
    pooltablemodel.h \
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
    prices.h \
    settings.h \
    collectiontablemodel.h \
    magiccollection.h \
    shortcuttype.h \
    deck.h \
    decktablemodel.h \
    deckmanager.h \
    statusbar.h \
    tableview.h \
    util.h \
    filters/timefilterfunction.h \
    filters/comparetype.h \
    filters/numberfilterfunction.h \
    deckstatisticsdialog.h \
    manacurvelinewidget.h \
    filters/deckfilterfunction.h \
    magicruling.h \
    splashscreen.h \
    tags.h \
    filters/containsfilterfunction.h \
    filters/tagfilterfunction.h

FORMS    += mainwindow.ui \
    deckwidget.ui \
    optionsdialog.ui \
    aboutdialog.ui \
    filtereditordialog.ui \
    deckstatisticsdialog.ui \
    manacurvelinewidget.ui \
    statusbar.ui

RESOURCES += \
    mtgcards.qrc

DISTFILES += \
    VERSION.txt
