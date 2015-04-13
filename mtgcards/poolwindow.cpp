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

	ui_.poolTbl_->setModel(&poolTableModel_);
	ui_.poolTbl_->horizontalHeader()->setSectionsMovable(true);
	ui_.poolTbl_->setSortingEnabled(true);
	ui_.poolTbl_->resizeColumnsToContents();

	connect(ui_.poolTbl_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(currentRowChanged(QModelIndex, QModelIndex)));

#if 0
	// test card picture availability
	for (int i = 0; i < poolTableModel_.rowCount(); ++i)
	{
		poolTableModel_.getPictureFilenames(i);
	}
#endif
}

PoolWindow::~PoolWindow()
{
}

void PoolWindow::reload()
{
	poolTableModel_.reload();
	ui_.poolTbl_->resizeColumnsToContents();
}

void PoolWindow::closeEvent(QCloseEvent* event)
{
	qDebug() << "Closing Pool Window";
	emit windowClosed(false);
	event->accept();
}

void PoolWindow::currentRowChanged(QModelIndex current, QModelIndex /*previous*/)
{
	qDebug() << "Changed to row " << current.row() << ")";
	auto mappedIdx = poolTableModel_.mapToSource(current);
	qDebug() << "Mapped row = " << mappedIdx.row();
	auto rv = poolTableModel_.getPictureFilenames(mappedIdx.row());
	emit selectCardChanged(rv.first, rv.second);
}
