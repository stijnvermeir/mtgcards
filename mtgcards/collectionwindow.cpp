#include "collectionwindow.h"

#include "magicitemdelegate.h"
#include "filtereditordialog.h"
#include "magiccollection.h"

#include <QSettings>
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
	connect(ui_.collectionTbl_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(currentRowChanged(QModelIndex, QModelIndex)));
	connect(ui_.actionAdvancedFilter, SIGNAL(triggered()), this, SLOT(actionAdvancedFilter()));
	connect(ui_.actionAddToCollection, SIGNAL(triggered()), this, SLOT(actionAddToCollection()));
	connect(ui_.actionRemoveFromCollection, SIGNAL(triggered()), this, SLOT(actionRemoveFromCollection()));
	connect(ui_.actionAddToDeck, SIGNAL(triggered()), this, SLOT(actionAddToDeck()));
	connect(ui_.actionRemoveFromDeck, SIGNAL(triggered()), this, SLOT(actionRemoveFromDeck()));
}

CollectionWindow::~CollectionWindow()
{
}

void CollectionWindow::reload()
{
	collectionTableModel_.reload();
}

void CollectionWindow::loadSettings()
{
	QSettings settings;
	if (settings.contains("collectionwindow/headerstate"))
	{
		if (!ui_.collectionTbl_->horizontalHeader()->restoreState(settings.value("collectionwindow/headerstate").toByteArray()))
		{
			qWarning() << "Failed to restore header state";
		}
	}
	else
	{
		ui_.collectionTbl_->resizeColumnsToContents();
	}
}

void CollectionWindow::saveSettings()
{
	QSettings settings;
	settings.setValue("collectionwindow/headerstate", ui_.collectionTbl_->horizontalHeader()->saveState());
}

void CollectionWindow::closeEvent(QCloseEvent* event)
{
	emit windowClosed(false);
	event->accept();
}

int CollectionWindow::currentDataRowIndex() const
{
	QModelIndex proxyIndex = ui_.collectionTbl_->currentIndex();
	QModelIndex sourceIndex = collectionTableModel_.mapToSource(proxyIndex);
	return mtg::Collection::instance().getDataRowIndex(sourceIndex.row());
}

QVector<int> CollectionWindow::currentDataRowIndices() const
{
	QModelIndexList list = ui_.collectionTbl_->selectionModel()->selectedRows();
	QVector<int> indices;
	indices.reserve(list.size());
	for (const auto& proxyIndex : list)
	{
		QModelIndex sourceIndex = collectionTableModel_.mapToSource(proxyIndex);
		indices.push_back(mtg::Collection::instance().getDataRowIndex(sourceIndex.row()));
	}
	return indices;
}

void CollectionWindow::addToCollection(const QVector<int>& dataRowIndices)
{
	for (const int dataRowIndex : dataRowIndices)
	{
		auto currentQuantity = collectionTableModel_.getQuantity(dataRowIndex);
		collectionTableModel_.setQuantity(dataRowIndex, currentQuantity + 1);
	}
	if (!dataRowIndices.empty())
	{
		mtg::Collection::instance().save();
		int rowIndex = mtg::Collection::instance().getRowIndex(dataRowIndices.back());
		if (rowIndex >= 0)
		{
			int columnIndex = ui_.collectionTbl_->horizontalHeader()->logicalIndexAt(0);
			QModelIndex sourceIndex = collectionTableModel_.sourceModel()->index(rowIndex, columnIndex);
			QModelIndex proxyIndex = collectionTableModel_.mapFromSource(sourceIndex);
			ui_.collectionTbl_->setCurrentIndex(proxyIndex);
		}
	}
}

void CollectionWindow::removeFromCollection(const QVector<int>& dataRowIndices)
{
	for (const int dataRowIndex : dataRowIndices)
	{
		int rowIndex = ui_.collectionTbl_->currentIndex().row();
		auto currentQuantity = collectionTableModel_.getQuantity(dataRowIndex);
		if (currentQuantity > 0)
		{
			collectionTableModel_.setQuantity(dataRowIndex, currentQuantity - 1);
			if (currentQuantity == 1) // last one removed --> row removed
			{
				if (rowIndex > 0) // check if it's not the top row
				{
					--rowIndex;
				}
				ui_.collectionTbl_->setCurrentIndex(collectionTableModel_.index(rowIndex, ui_.collectionTbl_->horizontalHeader()->logicalIndexAt(0)));
			}
			else
			{
				int rowIndex = mtg::Collection::instance().getRowIndex(dataRowIndex);
				int columnIndex = ui_.collectionTbl_->horizontalHeader()->logicalIndexAt(0);
				QModelIndex sourceIndex = collectionTableModel_.sourceModel()->index(rowIndex, columnIndex);
				QModelIndex proxyIndex = collectionTableModel_.mapFromSource(sourceIndex);
				ui_.collectionTbl_->setCurrentIndex(proxyIndex);
			}
		}
	}
	if (!dataRowIndices.empty())
	{
		mtg::Collection::instance().save();
	}
}

void CollectionWindow::currentRowChanged(QModelIndex, QModelIndex)
{
	if (ui_.collectionTbl_->hasFocus())
	{
		emit selectedCardChanged(currentDataRowIndex());
	}
}

void CollectionWindow::actionAdvancedFilter()
{
	FilterEditorDialog editor;
	editor.setFilterRootNode(rootFilterNode_);
	editor.exec();
	rootFilterNode_ = editor.getFilterRootNode();
	collectionTableModel_.setFilterRootNode(rootFilterNode_);
}

void CollectionWindow::actionAddToCollection()
{
	addToCollection(currentDataRowIndices());
}

void CollectionWindow::actionRemoveFromCollection()
{
	removeFromCollection(currentDataRowIndices());
}

void CollectionWindow::actionAddToDeck()
{
	emit addToDeck(currentDataRowIndices());
}

void CollectionWindow::actionRemoveFromDeck()
{
	emit removeFromDeck(currentDataRowIndices());
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
