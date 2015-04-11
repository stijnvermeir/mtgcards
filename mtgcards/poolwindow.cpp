#include "poolwindow.h"

#include <QSettings>
#include <QCloseEvent>
#include <QDebug>

PoolWindow::PoolWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, poolTableModel_()
{
	ui_.setupUi(this);

	QSettings settings;
	resize(settings.value("poolwindow/size", QSize(1428, 361)).toSize());
	move(settings.value("poolwindow/pos", QPoint(486, 0)).toPoint());

	ui_.poolTbl_->setModel(&poolTableModel_);
	ui_.poolTbl_->horizontalHeader()->setSectionsMovable(true);
	ui_.poolTbl_->setSortingEnabled(true);
	ui_.poolTbl_->resizeColumnsToContents();
}

PoolWindow::~PoolWindow()
{
}

void PoolWindow::closeEvent(QCloseEvent* event)
{
	qDebug() << "Closing Pool Window";
	QSettings settings;
	settings.setValue("poolwindow/size", size());
	settings.setValue("poolwindow/pos", pos());
	emit windowClosed(false);
	if (event)
	{
		event->accept();
	}
}
