#include "mainwindow.h"
#include "splashscreen.h"
#include "magiccarddata.h"
#include "magiccollection.h"
#include "prices.h"
#include "deckmanager.h"
#include "settings.h"
#include "util.h"
#include "tags.h"
#include "categories.h"

#include <QApplication>
#include <QWindow>
#include <QFile>
#include <QDebug>

#include <iostream>

using namespace std;

void myMessageOutput(QtMsgType type, const QMessageLogContext& /*context*/, const QString& msg)
{
	if (type < QtDebugMsg)
	{
		return;
	}
	cout << "[" << QDateTime::currentDateTime().toLocalTime().toString().toStdString() << "]";
	switch (type)
	{
		case QtDebugMsg:	cout << "(DBG)"; break;
        case QtInfoMsg:     cout << "(DBG)"; break;
		case QtWarningMsg:	cout << "(WRN)"; break;
		case QtCriticalMsg:	cout << "(ERR)"; break;
		case QtFatalMsg:	cout << "(FAT)"; break;
	}
	// cout << " " << msg.toStdString() << " [" << context.file << ":" << context.function << ":" << context.line << "]" << endl;
	cout << " " << msg.toStdString() << endl;
	cout << flush;
}

int main(int argc, char *argv[])
{
	qInstallMessageHandler(myMessageOutput);

	QCoreApplication::setOrganizationName("stijnvsoftware");
	QCoreApplication::setOrganizationDomain("stijn-v.be");
	QCoreApplication::setApplicationName("MTGCards");
	QCoreApplication::setApplicationVersion(MTGCARDS_VERSION);

	QApplication a(argc, argv);
	if (!QFile(Settings::instance().getPoolDataFile()).exists())
	{
		Util::downloadPoolDataFile();
	}
	SplashScreen splash;
	mtg::CardData::instance();
	a.processEvents();
	DeckManager::instance();
	a.processEvents();
	mtg::Collection::instance();
	a.processEvents();
	Prices::instance();
	a.processEvents();
	Tags::instance();
	a.processEvents();
	Categories::instance();
	a.processEvents();
	MainWindow mainWindow;
	a.processEvents();
	splash.finish(&mainWindow);
	mainWindow.show();
    return a.exec();
}
