#include "deckwidget.h"

#include "magicitemdelegate.h"
#include "deck.h"
#include "settings.h"
#include "util.h"
#include "magiccarddata.h"
#include "magiccollection.h"

#include <QMenu>
#include <QDebug>

namespace {

class DeckItemDelegate : public MagicItemDelegate
{
public:
	DeckItemDelegate(const DeckTableModel& model)
		: model_(model)
	{
	}

	virtual mtg::ColumnType columnIndexToType(const int columnIndex) const
	{
		return model_.columnIndexToType(columnIndex);
	}
private:
	const DeckTableModel& model_;
};

} // namespace

DeckWidget::DeckWidget(const QString& filename, CommonActions& commonActions, QAction& actionCopyDeckStatsClipboard, QWidget* parent)
	: QWidget(parent)
	, ui_()
	, deckTableModel_(filename)
	, itemDelegate_(new DeckItemDelegate(deckTableModel_))
	, headerStateChangedSlotDisabled_(false)
    , commonActions_(commonActions)
    , actionCopyDeckStatsClipboard_(actionCopyDeckStatsClipboard)
{
	ui_.setupUi(this);
	ui_.tableView->setItemDelegate(itemDelegate_.data());
	ui_.tableView->setModel(&deckTableModel_);
	ui_.tableView->setSortingEnabled(true);
	ui_.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui_.tableView->horizontalHeader()->setSectionsMovable(true);
	ui_.tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	ui_.tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui_.tableView->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(hideColumnsContextMenuRequested(QPoint)));
	connect(ui_.tableView->horizontalHeader(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(headerStateChangedSlot()));
	connect(ui_.tableView->horizontalHeader(), SIGNAL(sectionResized(int,int,int)), this, SLOT(headerStateChangedSlot()));
	connect(ui_.tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(headerStateChangedSlot()));
	connect(ui_.tableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(currentRowChanged(QModelIndex, QModelIndex)));
	connect(&deckTableModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)));
	connect(ui_.tableView, &TableView::customContextMenuRequested, this, &DeckWidget::rowContextMenuRequested);
	connect(this, SIGNAL(fontChanged()), ui_.tableView, SLOT(handleFontChanged()));

	connect(ui_.tableView, SIGNAL(searchStringChanged(QString)), this, SIGNAL(searchStringChanged(QString)));
}

DeckWidget::~DeckWidget()
{
}

void DeckWidget::save(const QString& filename)
{
	deckTableModel_.save(filename);
}

void DeckWidget::setDeckActive(const bool active)
{
	deckTableModel_.setDeckActive(active);
	emit deckEdited();
}

const Deck& DeckWidget::deck() const
{
	return deckTableModel_.deck();
}

void DeckWidget::setHeaderState(const QString& headerState)
{
	if (!headerState.isEmpty())
	{
		headerStateChangedSlotDisabled_ = true;
		Util::loadHeaderViewState(*ui_.tableView->horizontalHeader(), headerState);
		headerStateChangedSlotDisabled_ = false;
	}
}

void DeckWidget::setFilterRootNode(const FilterNode::Ptr& node)
{
	deckTableModel_.setFilterRootNode(node);
}

int DeckWidget::currentDataRowIndex() const
{
	return deckTableModel_.getDataRowIndex(ui_.tableView->currentIndex());
}

QVector<int> DeckWidget::currentDataRowIndices() const
{
	QModelIndexList list = ui_.tableView->selectionModel()->selectedRows();
	QVector<int> indices;
	indices.reserve(list.size());
	for (const auto& proxyIndex : list)
	{
		indices.push_back(deckTableModel_.getDataRowIndex(proxyIndex));
	}
	return indices;
}

const DeckTableModel& DeckWidget::model() const
{
	return deckTableModel_;
}

bool DeckWidget::isColumnHidden(const mtg::ColumnType& columnType) const
{
	return ui_.tableView->isColumnHidden(deckTableModel_.columnToIndex(columnType));
}

void DeckWidget::resetSearchString()
{
	ui_.tableView->resetKeyboardSearch();
}

void DeckWidget::addToDeck(const QVector<int>& dataRowIndices)
{
	for (const int dataRowIndex : dataRowIndices)
	{
		auto currentQuantity = deckTableModel_.deck().getQuantity(dataRowIndex);
		deckTableModel_.setQuantity(dataRowIndex, currentQuantity + 1);
	}
	if (!dataRowIndices.empty())
	{
		int rowIndex = deckTableModel_.getRowIndex(dataRowIndices.back());
		if (rowIndex >= 0)
		{
			int columnIndex = ui_.tableView->horizontalHeader()->logicalIndexAt(0);
			QModelIndex sourceIndex = deckTableModel_.sourceModel()->index(rowIndex, columnIndex);
			QModelIndex proxyIndex = deckTableModel_.mapFromSource(sourceIndex);
			ui_.tableView->setCurrentIndex(proxyIndex);
		}
		emit deckEdited();
	}
}

void DeckWidget::removeFromDeck(const QVector<int>& dataRowIndices)
{
	for (const int dataRowIndex : dataRowIndices)
	{
		int rowIndex = ui_.tableView->currentIndex().row();
		auto currentQuantity = deckTableModel_.deck().getQuantity(dataRowIndex);
		if (currentQuantity >= 0)
		{
			deckTableModel_.setQuantity(dataRowIndex, currentQuantity - 1);
			if (currentQuantity == 0) // last one removed --> row removed
			{
				if (rowIndex > 0) // check if it's not the top row
				{
					--rowIndex;
				}
				ui_.tableView->setCurrentIndex(deckTableModel_.index(rowIndex, ui_.tableView->horizontalHeader()->logicalIndexAt(0)));
			}
			else
			{
				int rowIndex = deckTableModel_.getRowIndex(dataRowIndex);
				int columnIndex = ui_.tableView->horizontalHeader()->logicalIndexAt(0);
				QModelIndex sourceIndex = deckTableModel_.sourceModel()->index(rowIndex, columnIndex);
				QModelIndex proxyIndex = deckTableModel_.mapFromSource(sourceIndex);
				ui_.tableView->setCurrentIndex(proxyIndex);
			}
		}
	}
	if (!dataRowIndices.empty())
	{
		emit deckEdited();
	}
}

void DeckWidget::downloadCardArt()
{
	deckTableModel_.downloadCardArt(ui_.tableView->selectionModel()->selectedRows());
}

void DeckWidget::fetchOnlineData()
{
	deckTableModel_.fetchOnlineData(ui_.tableView->selectionModel()->selectedRows());
}

void DeckWidget::currentRowChanged(QModelIndex, QModelIndex)
{
	if (ui_.tableView->hasFocus())
	{
		emit selectedCardChanged(currentDataRowIndex());
	}
}

void DeckWidget::dataChanged(QModelIndex, QModelIndex)
{
	emit deckEdited();
}

void DeckWidget::hideColumnsContextMenuRequested(const QPoint& pos)
{
	QMenu contextMenu(this);
	for (int i = 0; i < deckTableModel_.columnCount(); ++i)
	{
		QAction* action = new QAction(&contextMenu);
		action->setCheckable(true);
		action->setText(deckTableModel_.headerData(i, Qt::Horizontal).toString());
		action->setData(i);
		action->setChecked(!ui_.tableView->horizontalHeader()->isSectionHidden(i));
		contextMenu.addAction(action);
	}
	QAction* a = contextMenu.exec(ui_.tableView->horizontalHeader()->mapToGlobal(pos));
	if (a)
	{
		headerStateChangedSlotDisabled_ = true;
		ui_.tableView->horizontalHeader()->setSectionHidden(a->data().toInt(), !a->isChecked());
		headerStateChangedSlotDisabled_ = false;
		headerStateChangedSlot();
	}
}

void DeckWidget::rowContextMenuRequested(const QPoint& pos)
{
	QMenu contextMenu(ui_.tableView);
	auto indices = currentDataRowIndices();
	int dataRowIndex = -1;
	if (indices.size() == 1)
	{
		dataRowIndex = indices.at(0);
		int rowIndex = deck().getRowIndex(dataRowIndex);
		auto name = deck().get(rowIndex, mtg::ColumnType::Name).toString();
		auto reprints = mtg::CardData::instance().findRowsFast(name);
		if (reprints.size() > 1 && reprints.size() < 50)
		{
			QMenu* menu = contextMenu.addMenu("Change set");
			for (int i : reprints)
			{
				QAction* action = new QAction(menu);
				action->setCheckable(true);
				int owned = mtg::Collection::instance().getQuantity(i);
				int used = mtg::Collection::instance().getUsedCount(i);
				if (owned > 0)
				{
					action->setText(QString("%1 (Owned: %2 - Used: %3)").arg(mtg::CardData::instance().get(i, mtg::ColumnType::SetName).toString()).arg(owned).arg(used));
				}
				else
				{
					action->setText(mtg::CardData::instance().get(i, mtg::ColumnType::SetName).toString());
				}
				action->setData(i);
				action->setChecked(dataRowIndex == i);
				menu->addAction(action);
			}
			contextMenu.addSeparator();
		}
	}
	commonActions_.addToMenu(&contextMenu);
	contextMenu.addSeparator();
	contextMenu.addAction(&actionCopyDeckStatsClipboard_);
	QAction* a = contextMenu.exec(ui_.tableView->mapToGlobal(pos));
	if (a)
	{
		if (a->data().isValid())
		{
			int newDataRowIndex = a->data().toInt();
			if (newDataRowIndex != dataRowIndex)
			{
				int quantity = deck().getQuantity(dataRowIndex);
				int sb = deck().getSideboard(dataRowIndex);
				deckTableModel_.setQuantity(dataRowIndex, -1);
				deckTableModel_.setQuantity(newDataRowIndex, quantity);
				deckTableModel_.setSideboard(newDataRowIndex, sb);
				emit deckEdited();
			}
		}
	}
}

void DeckWidget::headerStateChangedSlot()
{
	if (headerStateChangedSlotDisabled_)
	{
		return;
	}
	headerStateChangedSlotDisabled_ = true;
	emit headerStateChangedSignal(Util::saveHeaderViewState(*ui_.tableView->horizontalHeader()));
	headerStateChangedSlotDisabled_ = false;
}
