#include "collectionwindow.h"

#include <QSettings>
#include <QCloseEvent>
#include <QDebug>

CollectionWindow::CollectionWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
{
	ui_.setupUi(this);

	QSettings settings;
	resize(settings.value("collectionwindow/size", QSize(1428, 367)).toSize());
	move(settings.value("collectionwindow/pos", QPoint(486, 390)).toPoint());
}

CollectionWindow::~CollectionWindow()
{
}

void CollectionWindow::closeEvent(QCloseEvent* event)
{
	qDebug() << "Closing Collection Window";
	QSettings settings;
	settings.setValue("collectionwindow/size", size());
	settings.setValue("collectionwindow/pos", pos());
	emit windowClosed(false);
	if (event)
	{
		event->accept();
	}
}

