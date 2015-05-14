#include "collectionwindow.h"

#include "magicitemdelegate.h"
#include "filtereditordialog.h"
#include "magiccollection.h"
#include "settings.h"
#include "deckmanager.h"
#include "util.h"

#include <QSettings>
#include <QCloseEvent>
#include <QMenu>
#include <QDebug>

namespace {

class CollectionItemDelegate : public MagicItemDelegate
{
public:
	CollectionItemDelegate(const CollectionTableModel& model)
		: model_(model)
	{
	}

	virtual mtg::ColumnType columnIndexToType(const int columnIndex) const
	{
		return model_.columnIndexToType(columnIndex);
	}
private:
	const CollectionTableModel& model_;
};

} // namespace

CollectionWindow::CollectionWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, collectionTableModel_()
	, itemDelegate_(new CollectionItemDelegate(collectionTableModel_))
	, rootFilterNode_()
{
	setWindowFlags(Qt::NoDropShadowWindowHint);
	ui_.setupUi(this);
	ui_.collectionTbl_->setItemDelegate(itemDelegate_.data());
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
	connect(&collectionTableModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)));
	connect(&DeckManager::instance(), SIGNAL(deckChanged()), this, SLOT(updateUsedCount()));

	QVector<UserColumn> userColumns;
	userColumns.append(UserColumn(UserColumn::DataType::Bool, "Some bool"));
	userColumns.append(UserColumn(UserColumn::DataType::Number, "Some number"));
	userColumns.append(UserColumn(UserColumn::DataType::Bool, "Another bool"));
	Settings::instance().setUserColumns(userColumns);
}

CollectionWindow::~CollectionWindow()
{
}

void CollectionWindow::reload()
{
	collectionTableModel_.reload();
	updateStatusBar();
}

void CollectionWindow::updateShortcuts()
{
	ui_.actionAdvancedFilter->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AdvancedFilter]);
	ui_.actionAddToCollection->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AddToCollection]);
	ui_.actionRemoveFromCollection->setShortcut(Settings::instance().getShortcuts()[ShortcutType::RemoveFromCollection]);
	ui_.actionAddToDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AddToDeck]);
	ui_.actionRemoveFromDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::RemoveFromDeck]);
}

void CollectionWindow::loadSettings()
{
	QSettings settings;
	if (settings.contains("collectionwindow/headerstate"))
	{
		Util::loadHeaderViewState(*ui_.collectionTbl_->horizontalHeader(), settings.value("collectionwindow/headerstate").toString());
	}
	else
	{
		ui_.collectionTbl_->resizeColumnsToContents();
	}
	if (settings.contains("collectionwindow/filter"))
	{
		rootFilterNode_ = FilterNode::createFromJson(QJsonDocument::fromJson(settings.value("collectionwindow/filter").toString().toUtf8()));
		collectionTableModel_.setFilterRootNode(rootFilterNode_);
	}
	updateStatusBar();
	updateShortcuts();
}

void CollectionWindow::saveSettings()
{
	QSettings settings;
	settings.setValue("collectionwindow/headerstate", Util::saveHeaderViewState(*ui_.collectionTbl_->horizontalHeader()));
	if (rootFilterNode_)
	{
		settings.setValue("collectionwindow/filter", QString(rootFilterNode_->toJson().toJson(QJsonDocument::Compact)));
	}
	else
	{
		settings.remove("collectionwindow/filter");
	}
}

void CollectionWindow::closeEvent(QCloseEvent* event)
{
	emit windowClosed(false);
	event->accept();
}

bool CollectionWindow::event(QEvent* event)
{
	if (event->type() == QEvent::WindowActivate /*|| event->type() == QEvent::Enter*/)
	{
		emit selectedCardChanged(currentDataRowIndex());
	}
	return QMainWindow::event(event);
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

void CollectionWindow::updateStatusBar()
{
	QString message;
	QTextStream stream(&message);
	stream << " Showing " << collectionTableModel_.rowCount() << " of " << mtg::Collection::instance().getNumRows() << " cards";
	int numCopies = 0;
	for (int i = 0; i < collectionTableModel_.rowCount(); ++i)
	{
		QModelIndex sourceIndex = collectionTableModel_.mapToSource(collectionTableModel_.index(i, 0));
		numCopies += mtg::Collection::instance().get(sourceIndex.row(), mtg::ColumnType::Quantity).toInt();
	}
	stream << " (" << numCopies << " copies)";
	ui_.statusBar->showMessage(message);
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
	updateStatusBar();
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
	updateStatusBar();
}

void CollectionWindow::currentRowChanged(QModelIndex, QModelIndex)
{
	if (ui_.collectionTbl_->hasFocus())
	{
		emit selectedCardChanged(currentDataRowIndex());
	}
}

void CollectionWindow::dataChanged(QModelIndex, QModelIndex)
{
	updateStatusBar();
}

void CollectionWindow::actionAdvancedFilter()
{
	FilterEditorDialog editor;
	editor.setFilterRootNode(rootFilterNode_);
	editor.exec();
	rootFilterNode_ = editor.getFilterRootNode();
	collectionTableModel_.setFilterRootNode(rootFilterNode_);
	updateStatusBar();
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

void CollectionWindow::updateUsedCount()
{
	for (int row = 0; row < mtg::Collection::instance().getNumRows(); ++row)
	{
		int usedCount = DeckManager::instance().getUsedCount(mtg::Collection::instance().getDataRowIndex(row));
		int usedColumnIndex = collectionTableModel_.columnToIndex(mtg::ColumnType::Used);
		QModelIndex sourceIndex = collectionTableModel_.sourceModel()->index(row, usedColumnIndex);
		collectionTableModel_.sourceModel()->setData(sourceIndex, usedCount);
	}
}
