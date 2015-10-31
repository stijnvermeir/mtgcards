#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>

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
	MyApplication(int &argc, char **argv)
		: QApplication(argc, argv)
		, mainWindow_()
	{
	}

protected:
#ifdef __APPLE__
	virtual bool event(QEvent* e)
	{
		if (e->type() == QEvent::Close)
		{
			return mainWindow_.toQuitOrNotToQuit(e);
		}
		return QApplication::event(e);
	}
#endif

private:
	MainWindow mainWindow_;
};

int main(int argc, char *argv[])
{
	qInstallMessageHandler(myMessageOutput);

	QCoreApplication::setOrganizationName("stijnvsoftware");
	QCoreApplication::setOrganizationDomain("stijn-v.be");
	QCoreApplication::setApplicationName("MTGCards");
	QCoreApplication::setApplicationVersion("0.2.0");
	QGuiApplication::setQuitOnLastWindowClosed(false);

	MyApplication a(argc, argv);
    return a.exec();
}
