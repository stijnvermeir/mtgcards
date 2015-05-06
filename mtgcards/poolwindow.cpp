#include "poolwindow.h"

#include "magicitemdelegate.h"
#include "filtereditordialog.h"

#include <QSettings>
#include <QCloseEvent>
#include <QMenu>
#include <QDebug>

PoolWindow::PoolWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, poolTableModel_()
	, rootFilterNode_()
{
	ui_.setupUi(this);
	ui_.poolTbl_->setItemDelegate(new MagicItemDelegate());
	ui_.poolTbl_->setModel(&poolTableModel_);
	ui_.poolTbl_->setSortingEnabled(true);
	ui_.poolTbl_->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui_.poolTbl_->horizontalHeader()->setSectionsMovable(true);
	ui_.poolTbl_->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui_.poolTbl_->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(hideColumnsContextMenuRequested(QPoint)));
	connect(ui_.poolTbl_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(currentRowChanged(QModelIndex, QModelIndex)));
	connect(ui_.actionAdvancedFilter, SIGNAL(triggered()), this, SLOT(actionAdvancedFilter()));
	connect(ui_.actionAddToCollection, SIGNAL(triggered()), this, SLOT(actionAddToCollection()));
	connect(ui_.actionRemoveFromCollection, SIGNAL(triggered()), this, SLOT(actionRemoveFromCollection()));
	connect(ui_.actionAddToDeck, SIGNAL(triggered()), this, SLOT(actionAddToDeck()));
	connect(ui_.actionRemoveFromDeck, SIGNAL(triggered()), this, SLOT(actionRemoveFromDeck()));
}

PoolWindow::~PoolWindow()
{
}

void PoolWindow::reload()
{
	poolTableModel_.reload();
}

void PoolWindow::loadSettings()
{
	QSettings settings;
	if (settings.contains("poolwindow/headerstate"))
	{
		if (!ui_.poolTbl_->horizontalHeader()->restoreState(settings.value("poolwindow/headerstate").toByteArray()))
		{
			qWarning() << "Failed to restore header state";
		}
	}
	else
	{
		ui_.poolTbl_->resizeColumnsToContents();
	}
}

void PoolWindow::saveSettings()
{
	QSettings settings;
	settings.setValue("poolwindow/headerstate", ui_.poolTbl_->horizontalHeader()->saveState());
}

void PoolWindow::closeEvent(QCloseEvent* event)
{
	emit windowClosed(false);
	event->accept();
}

int PoolWindow::currentDataRowIndex() const
{
	QModelIndex proxyIndex = ui_.poolTbl_->currentIndex();
	QModelIndex sourceIndex = poolTableModel_.mapToSource(proxyIndex);
	return sourceIndex.row();
}

QVector<int> PoolWindow::currentDataRowIndices() const
{
	QModelIndexList list = ui_.poolTbl_->selectionModel()->selectedRows();
	QVector<int> indices;
	indices.reserve(list.size());
	for (const auto& proxyIndex : list)
	{
		QModelIndex sourceIndex = poolTableModel_.mapToSource(proxyIndex);
		indices.push_back(sourceIndex.row());
	}
	return indices;
}

void PoolWindow::currentRowChanged(QModelIndex, QModelIndex)
{
	emit selectedCardChanged(currentDataRowIndex());
}

void PoolWindow::actionAdvancedFilter()
{
	FilterEditorDialog editor;
	editor.setFilterRootNode(rootFilterNode_);
	editor.exec();
	rootFilterNode_ = editor.getFilterRootNode();
	poolTableModel_.setFilterRootNode(rootFilterNode_);
}

void PoolWindow::actionAddToCollection()
{
	emit addToCollection(currentDataRowIndices());
}

void PoolWindow::actionRemoveFromCollection()
{
	emit removeFromCollection(currentDataRowIndices());
}

void PoolWindow::actionAddToDeck()
{
	emit addToDeck(currentDataRowIndices());
}

void PoolWindow::actionRemoveFromDeck()
{
	emit removeFromDeck(currentDataRowIndices());
}

void PoolWindow::hideColumnsContextMenuRequested(const QPoint& pos)
{
	QMenu contextMenu(this);
	for (int i = 0; i < poolTableModel_.columnCount(); ++i)
	{
		QAction* action = new QAction(&contextMenu);
		action->setCheckable(true);
		action->setText(poolTableModel_.headerData(i, Qt::Horizontal).toString());
		action->setData(i);
		action->setChecked(!ui_.poolTbl_->horizontalHeader()->isSectionHidden(i));
		contextMenu.addAction(action);
	}
	QAction* a = contextMenu.exec(ui_.poolTbl_->horizontalHeader()->mapToGlobal(pos));
	if (a)
	{
		ui_.poolTbl_->horizontalHeader()->setSectionHidden(a->data().toInt(), !a->isChecked());
	}
}
