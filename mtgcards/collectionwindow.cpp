#include "collectionwindow.h"

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
	emit windowClosed(false);
	event->accept();
}

