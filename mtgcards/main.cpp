#include "mainwindow.h"
#include "splashscreen.h"
#include "magiccarddata.h"
#include "magiccollection.h"
#include "deckmanager.h"
#include "onlinedatacache.h"
#include "settings.h"
#include "util.h"
#include "tags.h"

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
		case QtWarningMsg:	cout << "(WRN)"; break;
		case QtCriticalMsg:	cout << "(ERR)"; break;
		case QtFatalMsg:	cout << "(FAT)"; break;
	}
	// cout << " " << msg.toStdString() << " [" << context.file << ":" << context.function << ":" << context.line << "]" << endl;
	cout << " " << msg.toStdString() << endl;
	cout << flush;
}

class MyApplication : public QApplication
{
public:
	MainWindow* mainWindow_;

	MyApplication(int &argc, char **argv)
		: QApplication(argc, argv)
		, mainWindow_(nullptr)
	{
	}

#ifdef Q_OS_OSX
	virtual bool event(QEvent* e)
	{
		if (e->type() == QEvent::Close && mainWindow_)
		{
			return mainWindow_->toQuitOrNotToQuit(e);
		}
		return QApplication::event(e);
	}
#endif
};

int main(int argc, char *argv[])
{
	qInstallMessageHandler(myMessageOutput);

	QCoreApplication::setOrganizationName("stijnvsoftware");
	QCoreApplication::setOrganizationDomain("stijn-v.be");
	QCoreApplication::setApplicationName("MTGCards");
	QCoreApplication::setApplicationVersion(MTGCARDS_VERSION);
	QGuiApplication::setQuitOnLastWindowClosed(false);

	qDebug() << "SSL runtime: " << QSslSocket::sslLibraryVersionString();
	qDebug() << "SSL build:   " << QSslSocket::sslLibraryBuildVersionString();

	MyApplication a(argc, argv);
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
	OnlineDataCache::instance();
	a.processEvents();
	Tags::instance();
	a.processEvents();
	MainWindow mainWindow;
	a.processEvents();
	a.mainWindow_ = &mainWindow;
	splash.finish(a.mainWindow_);

    return a.exec();
}
