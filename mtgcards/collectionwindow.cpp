#include "collectionwindow.h"

#include "magicitemdelegate.h"
#include "filtereditordialog.h"

#include <QCloseEvent>
#include <QMenu>
#include <QDebug>

CollectionWindow::CollectionWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, collectionTableModel_()
	, rootFilterNode_()
{
	ui_.setupUi(this);

	ui_.collectionTbl_->setItemDelegate(new MagicItemDelegate());
	ui_.collectionTbl_->setModel(&collectionTableModel_);
	ui_.collectionTbl_->setSortingEnabled(true);
	ui_.collectionTbl_->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui_.collectionTbl_->horizontalHeader()->setSectionsMovable(true);

	ui_.collectionTbl_->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui_.collectionTbl_->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(hideColumnsContextMenuRequested(QPoint)));
}

CollectionWindow::~CollectionWindow()
{
}

void CollectionWindow::closeEvent(QCloseEvent* event)
{
	emit windowClosed(false);
	event->accept();
}

void CollectionWindow::hideColumnsContextMenuRequested(const QPoint& pos)
{
	QMenu contextMenu(this);
	for (int i = 0; i < collectionTableModel_.columnCount(); ++i)
	{
		QAction* action = new QAction(&contextMenu);
		action->setCheckable(true);
		action->setText(collectionTableModel_.headerData(i, Qt::Horizontal).toString());
		action->setData(i);
		action->setChecked(!ui_.collectionTbl_->horizontalHeader()->isSectionHidden(i));
		contextMenu.addAction(action);
	}
	QAction* a = contextMenu.exec(ui_.collectionTbl_->horizontalHeader()->mapToGlobal(pos));
	if (a)
	{
		ui_.collectionTbl_->horizontalHeader()->setSectionHidden(a->data().toInt(), !a->isChecked());
	}
}
