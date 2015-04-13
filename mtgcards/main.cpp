#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>

#include <iostream>

using namespace std;

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
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
	cout << " " << msg.toStdString() << " [" << context.file << ":" << context.function << ":" << context.line << "]" << endl;
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
	virtual bool event(QEvent* e)
	{
		qDebug() << "Event type = " << e->type();
		if (e->type() == QEvent::Close)
		{
			mainWindow_.saveSettings();
		}
		return QApplication::event(e);
	}

private:
	MainWindow mainWindow_;
};

int main(int argc, char *argv[])
{
	qInstallMessageHandler(myMessageOutput);

	QCoreApplication::setOrganizationName("stijnvsoftware");
	QCoreApplication::setOrganizationDomain("stijn-v.be");
	QCoreApplication::setApplicationName("MTGCards");
	QCoreApplication::setApplicationVersion("0.0.1");
	QGuiApplication::setQuitOnLastWindowClosed(false);

	MyApplication a(argc, argv);
    return a.exec();
}

