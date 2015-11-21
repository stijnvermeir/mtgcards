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

DEFINES += "MTGCARDS_VERSION=\"\\\"$$cat(VERSION.txt)\\\"\""

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
    magiccollection.cpp \
    shortcuttype.cpp \
    deck.cpp \
    decktablemodel.cpp \
    deckmanager.cpp \
    tableview.cpp \
    util.cpp \
    usercolumn.cpp \
    filters/timefilterfunction.cpp \
    filters/comparetype.cpp \
    filters/numberfilterfunction.cpp \
    deckstatisticsdialog.cpp \
    manacurvelinewidget.cpp \
    filters/deckfilterfunction.cpp \
    onlinedatacache.cpp \
    splashscreen.cpp \
    addtowantslistdialog.cpp

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
    magiccollection.h \
    shortcuttype.h \
    deck.h \
    decktablemodel.h \
    deckmanager.h \
    tableview.h \
    util.h \
    usercolumn.h \
    filters/timefilterfunction.h \
    filters/comparetype.h \
    filters/numberfilterfunction.h \
    deckstatisticsdialog.h \
    manacurvelinewidget.h \
    filters/deckfilterfunction.h \
    onlinedatacache.h \
    magicruling.h \
    splashscreen.h \
    addtowantslistdialog.h

FORMS    += mainwindow.ui \
    poolwindow.ui \
    cardwindow.ui \
    collectionwindow.ui \
    deckwindow.ui \
    deckwidget.ui \
    optionsdialog.ui \
    aboutdialog.ui \
    filtereditordialog.ui \
    deckstatisticsdialog.ui \
    manacurvelinewidget.ui \
    addtowantslistdialog.ui

RESOURCES += \
    mtgcards.qrc

MKMLIB_PATH = $$PWD/../../mkmlib

CONFIG(release, debug|release): LIBS += -L$$MKMLIB_PATH/build/release/
else:CONFIG(debug, debug|release): LIBS += -L$$MKMLIB_PATH/build/debug/

LIBS += -lmkm

INCLUDEPATH += $$MKMLIB_PATH/mkm
DEPENDPATH += $$MKMLIB_PATH/mkm

DISTFILES += \
    VERSION.txt
