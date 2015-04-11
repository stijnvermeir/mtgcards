#include "cardwindow.h"

#include <QSettings>
#include <QCloseEvent>
#include <QDebug>

CardWindow::CardWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
{
	ui_.setupUi(this);

	QSettings settings;
	resize(settings.value("cardwindow/size", QSize(480, 680)).toSize());
	move(settings.value("cardwindow/pos", QPoint(0, 77)).toPoint());
}

CardWindow::~CardWindow()
{
}

void CardWindow::closeEvent(QCloseEvent* event)
{
	qDebug() << "Closing Card Window";
	QSettings settings;
	settings.setValue("cardwindow/size", size());
	settings.setValue("cardwindow/pos", pos());
	emit windowClosed(false);
	if (event)
	{
		event->accept();
	}
}
