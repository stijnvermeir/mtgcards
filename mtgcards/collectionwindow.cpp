#include "collectionwindow.h"

#include <QSettings>
#include <QCloseEvent>
#include <QDebug>

CollectionWindow::CollectionWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
{
	ui_.setupUi(this);
}

CollectionWindow::~CollectionWindow()
{
}

void CollectionWindow::closeEvent(QCloseEvent* event)
{
	qDebug() << "Closing Collection Window";
	emit windowClosed(false);
	event->accept();
}

