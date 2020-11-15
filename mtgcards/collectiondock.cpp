#include "collectiondock.h"

#include "magicitemdelegate.h"
#include "filtereditordialog.h"
#include "magiccollection.h"
#include "settings.h"
#include "deckmanager.h"
#include "util.h"

#include <QSettings>
#include <QMenu>
#include <QLabel>
#include <QDebug>

namespace {

const QString DEFAULT_HEADER_STATE = "{\"sections\": [{\"hidden\": true,\"size\": 100,\"visualIndex\": 0},{\"hidden\": false,\"size\": 61,\"visualIndex\": 3},{\"hidden\": true,\"size\": 100,\"visualIndex\": 1},{\"hidden\": true,\"size\": 100,\"visualIndex\": 2},{\"hidden\": true,\"size\": 100,\"visualIndex\": 4},{\"hidden\": true,\"size\": 294,\"visualIndex\": 5},{\"hidden\": true,\"size\": 100,\"visualIndex\": 6},{\"hidden\": true,\"size\": 37,\"visualIndex\": 7},{\"hidden\": true,\"size\": 37,\"visualIndex\": 8},{\"hidden\": false,\"size\": 264,\"visualIndex\": 9},{\"hidden\": true,\"size\": 233,\"visualIndex\": 10},{\"hidden\": false,\"size\": 38,\"visualIndex\": 11},{\"hidden\": false,\"size\": 41,\"visualIndex\": 12},{\"hidden\": false,\"size\": 218,\"visualIndex\": 13},{\"hidden\": false,\"size\": 41,\"visualIndex\": 14},{\"hidden\": false,\"size\": 77,\"visualIndex\": 15},{\"hidden\": false,\"size\": 287,\"visualIndex\": 16},{\"hidden\": true,\"size\": 47,\"visualIndex\": 17},{\"hidden\": true,\"size\": 100,\"visualIndex\": 18},{\"hidden\": true,\"size\": 100,\"visualIndex\": 19},{\"hidden\": false,\"size\": 72,\"visualIndex\": 22},{\"hidden\": true,\"size\": 86,\"visualIndex\": 23},{\"hidden\": true,\"size\": 100,\"visualIndex\": 24},{\"hidden\": true,\"size\": 100,\"visualIndex\": 25},{\"hidden\": false,\"size\": 20,\"visualIndex\": 20},{\"hidden\": false,\"size\": 20,\"visualIndex\": 21},{\"hidden\": true,\"size\": 100,\"visualIndex\": 26},{\"hidden\": true,\"size\": 100,\"visualIndex\": 27},{\"hidden\": true,\"size\": 100,\"visualIndex\": 28},{\"hidden\": false,\"size\": 107,\"visualIndex\": 29},{\"hidden\": true,\"size\": 111,\"visualIndex\": 30},{\"hidden\": true,\"size\": 85,\"visualIndex\": 31},{\"hidden\": false,\"size\": 79,\"visualIndex\": 32},{\"hidden\": true,\"size\": 62,\"visualIndex\": 33}],\"sortIndicatorOrder\": 0,\"sortIndicatorSection\": 9}";

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

CollectionDock::CollectionDock(Ui::MainWindow& ui, QWidget* parent)
    : QObject(parent)
    , ui_(ui)
	, collectionTableModel_()
	, itemDelegate_(new CollectionItemDelegate(collectionTableModel_))
	, rootFilterNode_()
{
	ui_.collectionTableView->setItemDelegate(itemDelegate_.data());
	ui_.collectionTableView->setModel(&collectionTableModel_);
	ui_.collectionTableView->setSortingEnabled(true);
	ui_.collectionTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui_.collectionTableView->horizontalHeader()->setSectionsMovable(true);
	ui_.collectionTableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	ui_.collectionTableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui_.collectionTableView->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(hideColumnsContextMenuRequested(QPoint)));
	connect(ui_.collectionTableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(rowContextMenuRequested(QPoint)));
	connect(ui_.collectionTableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(currentRowChanged(QModelIndex, QModelIndex)));
	connect(&collectionTableModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)));
	connect(&DeckManager::instance(), SIGNAL(deckChanged()), this, SLOT(updateUsedCount()));
	connect(this, SIGNAL(fontChanged()), ui_.collectionTableView, SLOT(handleFontChanged()));
	connect(ui_.collectionTableView, SIGNAL(searchStringChanged(QString)), ui_.collectionStatusBar, SLOT(setSearch(QString)));
	commonActions_.connectSignals(this);
	commonActions_.addToWidget(ui_.collectionTableView);

	ui_.collectionStatusBar->setViewChangerEnabled(false);
}

CollectionDock::~CollectionDock()
{
}

void CollectionDock::reload()
{
	collectionTableModel_.reload();
	updateStatusBar();
}

void CollectionDock::updateShortcuts()
{
	commonActions_.updateShortcuts();
}

void CollectionDock::loadSettings()
{
	QSettings settings;
	Util::loadHeaderViewState(*ui_.collectionTableView->horizontalHeader(), settings.value("collectionwindow/headerstate", DEFAULT_HEADER_STATE).toString());
	if (settings.contains("collectionwindow/filterEnable"))
	{
		commonActions_.getEnableFilter()->setChecked(settings.value("collectionwindow/filterEnable").toBool());
	}
	if (settings.contains("collectionwindow/filter"))
	{
		rootFilterNode_ = FilterNode::createFromJson(QJsonDocument::fromJson(settings.value("collectionwindow/filter").toString().toUtf8()));
		if (commonActions_.getEnableFilter()->isChecked())
		{
			collectionTableModel_.setFilterRootNode(rootFilterNode_);
		}
	}
	updateStatusBar();
	updateShortcuts();
}

void CollectionDock::saveSettings()
{
	QSettings settings;
	settings.setValue("collectionwindow/headerstate", Util::saveHeaderViewState(*ui_.collectionTableView->horizontalHeader()));
	settings.setValue("collectionwindow/filterEnable", commonActions_.getEnableFilter()->isChecked());
	if (rootFilterNode_)
	{
		settings.setValue("collectionwindow/filter", QString(rootFilterNode_->toJson().toJson(QJsonDocument::Compact)));
	}
	else
	{
		settings.remove("collectionwindow/filter");
	}
}

int CollectionDock::currentDataRowIndex() const
{
	QModelIndex proxyIndex = ui_.collectionTableView->currentIndex();
	QModelIndex sourceIndex = collectionTableModel_.mapToSource(proxyIndex);
	return mtg::Collection::instance().getDataRowIndex(sourceIndex.row());
}

QVector<int> CollectionDock::currentDataRowIndices() const
{
	QModelIndexList list = ui_.collectionTableView->selectionModel()->selectedRows();
	QVector<int> indices;
	indices.reserve(list.size());
	for (const auto& proxyIndex : list)
	{
		QModelIndex sourceIndex = collectionTableModel_.mapToSource(proxyIndex);
		indices.push_back(mtg::Collection::instance().getDataRowIndex(sourceIndex.row()));
	}
	return indices;
}

void CollectionDock::updateStatusBar()
{
	QString message;
	QTextStream stream(&message);
	stream << " Showing " << collectionTableModel_.rowCount() << " of " << mtg::Collection::instance().getNumRows() << " cards";
	int numCopies = 0;
	int numUsed = 0;
	double sumPriceLowest = 0.0;
	double sumPriceLowestFoil = 0.0;
	double sumPriceAverage = 0.0;
	double sumPriceTrend = 0.0;
	auto getValue = [this](int row, const mtg::ColumnType& columnType)
	{
		int column = collectionTableModel_.columnToIndex(columnType);
		QModelIndex index = collectionTableModel_.index(row, column);
		return collectionTableModel_.data(index);
	};
	for (int i = 0; i < collectionTableModel_.rowCount(); ++i)
	{
		QModelIndex sourceIndex = collectionTableModel_.mapToSource(collectionTableModel_.index(i, 0));
		auto qty = mtg::Collection::instance().get(sourceIndex.row(), mtg::ColumnType::Quantity).toInt();
		numCopies += qty;
		numUsed += mtg::Collection::instance().get(sourceIndex.row(), mtg::ColumnType::Used).toInt();

		sumPriceLowest += qty * getValue(i, mtg::ColumnType::PriceLowest).toDouble();
		sumPriceLowestFoil += qty * getValue(i, mtg::ColumnType::PriceLowestFoil).toDouble();
		sumPriceAverage += qty * getValue(i, mtg::ColumnType::PriceAverage).toDouble();
		sumPriceTrend += qty * getValue(i, mtg::ColumnType::PriceTrend).toDouble();
	}
	stream << " (" << numCopies << " copies, " << numUsed << " used)";
	if (!ui_.collectionTableView->isColumnHidden(collectionTableModel_.columnToIndex(mtg::ColumnType::PriceLowest)))
	{
		stream << " [Sum lowest price: " << sumPriceLowest << "]";
	}
	if (!ui_.collectionTableView->isColumnHidden(collectionTableModel_.columnToIndex(mtg::ColumnType::PriceLowestFoil)))
	{
		stream << " [Sum lowest price foil: " << sumPriceLowestFoil << "]";
	}
	if (!ui_.collectionTableView->isColumnHidden(collectionTableModel_.columnToIndex(mtg::ColumnType::PriceAverage)))
	{
		stream << " [Sum average price: " << sumPriceAverage << "]";
	}
	if (!ui_.collectionTableView->isColumnHidden(collectionTableModel_.columnToIndex(mtg::ColumnType::PriceTrend)))
	{
		stream << " [Sum price trend: " << sumPriceTrend << "]";
	}
	ui_.collectionStatusBar->setMessage(message);
}

void CollectionDock::addToCollection(const QVector<int>& dataRowIndices)
{
	for (const int dataRowIndex : dataRowIndices)
	{
		auto currentQuantity = collectionTableModel_.getQuantity(dataRowIndex);
		collectionTableModel_.setQuantity(dataRowIndex, currentQuantity + 1);
	}
	if (!dataRowIndices.empty())
	{
		int rowIndex = mtg::Collection::instance().getRowIndex(dataRowIndices.back());
		if (rowIndex >= 0)
		{
			int columnIndex = ui_.collectionTableView->horizontalHeader()->logicalIndexAt(0);
			QModelIndex sourceIndex = collectionTableModel_.sourceModel()->index(rowIndex, columnIndex);
			QModelIndex proxyIndex = collectionTableModel_.mapFromSource(sourceIndex);
			ui_.collectionTableView->setCurrentIndex(proxyIndex);
		}
	}
	updateStatusBar();
}

void CollectionDock::addToCollection(const QVector<QPair<int, int>>& additions)
{
	for (const QPair<int,int>& add : additions)
	{
		auto currentQuantity = collectionTableModel_.getQuantity(add.first);
		collectionTableModel_.setQuantity(add.first, currentQuantity + add.second);
	}
	if (!additions.empty())
	{
		int rowIndex = mtg::Collection::instance().getRowIndex(additions.back().first);
		if (rowIndex >= 0)
		{
			int columnIndex = ui_.collectionTableView->horizontalHeader()->logicalIndexAt(0);
			QModelIndex sourceIndex = collectionTableModel_.sourceModel()->index(rowIndex, columnIndex);
			QModelIndex proxyIndex = collectionTableModel_.mapFromSource(sourceIndex);
			ui_.collectionTableView->setCurrentIndex(proxyIndex);
		}
	}
	updateStatusBar();
}

void CollectionDock::removeFromCollection(const QVector<int>& dataRowIndices)
{
	for (const int dataRowIndex : dataRowIndices)
	{
		int rowIndex = ui_.collectionTableView->currentIndex().row();
		auto currentQuantity = collectionTableModel_.getQuantity(dataRowIndex);
		if (currentQuantity >= 0)
		{
			collectionTableModel_.setQuantity(dataRowIndex, currentQuantity - 1);
			if (currentQuantity == 0) // last one removed --> row removed
			{
				if (rowIndex > 0) // check if it's not the top row
				{
					--rowIndex;
				}
				ui_.collectionTableView->setCurrentIndex(collectionTableModel_.index(rowIndex, ui_.collectionTableView->horizontalHeader()->logicalIndexAt(0)));
			}
			else
			{
				int rowIndex = mtg::Collection::instance().getRowIndex(dataRowIndex);
				int columnIndex = ui_.collectionTableView->horizontalHeader()->logicalIndexAt(0);
				QModelIndex sourceIndex = collectionTableModel_.sourceModel()->index(rowIndex, columnIndex);
				QModelIndex proxyIndex = collectionTableModel_.mapFromSource(sourceIndex);
				ui_.collectionTableView->setCurrentIndex(proxyIndex);
			}
		}
	}
	updateStatusBar();
}

void CollectionDock::currentRowChanged(QModelIndex, QModelIndex)
{
	if (ui_.collectionTableView->hasFocus())
	{
		emit selectedCardChanged(currentDataRowIndex());
	}
}

void CollectionDock::dataChanged(QModelIndex, QModelIndex)
{
	updateStatusBar();
}

void CollectionDock::actionAdvancedFilter()
{
	FilterEditorDialog editor;
	editor.setFilterRootNode(rootFilterNode_);
	editor.exec();
	rootFilterNode_ = editor.getFilterRootNode();
	if (commonActions_.getEnableFilter()->isChecked())
	{
		collectionTableModel_.setFilterRootNode(rootFilterNode_);
	}
	updateStatusBar();
}

void CollectionDock::actionEnableFilter(bool enable)
{
	if (enable)
	{
		collectionTableModel_.setFilterRootNode(rootFilterNode_);
	}
	else
	{
		collectionTableModel_.setFilterRootNode(FilterNode::Ptr());
	}
	updateStatusBar();
}

void CollectionDock::actionAddToCollection()
{
	addToCollection(currentDataRowIndices());
}

void CollectionDock::actionRemoveFromCollection()
{
	removeFromCollection(currentDataRowIndices());
}

void CollectionDock::actionAddToDeck()
{
	emit addToDeck(currentDataRowIndices());
}

void CollectionDock::actionRemoveFromDeck()
{
	emit removeFromDeck(currentDataRowIndices());
}

void CollectionDock::actionDownloadCardArt()
{
	collectionTableModel_.downloadCardArt(ui_.collectionTableView->selectionModel()->selectedRows());
}

void CollectionDock::actionFetchOnlineData()
{
	collectionTableModel_.fetchOnlineData(ui_.collectionTableView->selectionModel()->selectedRows());
}

void CollectionDock::hideColumnsContextMenuRequested(const QPoint& pos)
{
	QMenu contextMenu(ui_.collectionTableView);
	for (int i = 0; i < collectionTableModel_.columnCount(); ++i)
	{
		QAction* action = new QAction(&contextMenu);
		action->setCheckable(true);
		action->setText(collectionTableModel_.headerData(i, Qt::Horizontal).toString());
		action->setData(i);
		action->setChecked(!ui_.collectionTableView->horizontalHeader()->isSectionHidden(i));
		contextMenu.addAction(action);
	}
	QAction* a = contextMenu.exec(ui_.collectionTableView->horizontalHeader()->mapToGlobal(pos));
	if (a)
	{
		ui_.collectionTableView->horizontalHeader()->setSectionHidden(a->data().toInt(), !a->isChecked());
		updateStatusBar();
	}
}

void CollectionDock::rowContextMenuRequested(const QPoint& pos)
{
	if (ui_.collectionTableView->currentIndex().isValid())
	{
		QMenu contextMenu(ui_.collectionTableView);
		QAction* openDecksAction = contextMenu.addAction("Open decks where this card is used");
		contextMenu.addSeparator();
		commonActions_.addToMenu(&contextMenu);
		QAction* a = contextMenu.exec(ui_.collectionTableView->mapToGlobal(pos));
		if (a == openDecksAction)
		{
			QModelIndex sourceIndex = collectionTableModel_.mapToSource(ui_.collectionTableView->currentIndex());
			int dataRowIndex = mtg::Collection::instance().getDataRowIndex(sourceIndex.row());
			auto decks = DeckManager::instance().getDecksUsedIn(dataRowIndex);
			for (const auto& deck : decks)
			{
				emit requestOpenDeck(deck->getId());
			}
		}
	}
}

void CollectionDock::updateUsedCount()
{
	collectionTableModel_.updateUsedCount();
}

void CollectionDock::handleGlobalFilterChanged()
{
	if (commonActions_.getEnableFilter()->isChecked())
	{
		collectionTableModel_.setFilterRootNode(rootFilterNode_);
	}
	else
	{
		collectionTableModel_.setFilterRootNode(FilterNode::Ptr());
	}
	updateStatusBar();
}
