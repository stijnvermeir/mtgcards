#include "deckwidget.h"

#include "magicitemdelegate.h"
#include "settings.h"

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

DeckWidget::DeckWidget(const QString& filename, QWidget *parent)
	: QWidget(parent)
	, ui_()
	, deckTableModel_(filename)
	, itemDelegate_(new DeckItemDelegate(deckTableModel_))
{
	ui_.setupUi(this);
	ui_.tableView->setItemDelegate(itemDelegate_.data());
	ui_.tableView->setModel(&deckTableModel_);
	ui_.tableView->setSortingEnabled(true);
	ui_.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui_.tableView->horizontalHeader()->setSectionsMovable(true);
	ui_.tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui_.tableView->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(hideColumnsContextMenuRequested(QPoint)));
	connect(ui_.tableView->horizontalHeader(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(headerStateChangedSlot()));
	connect(ui_.tableView->horizontalHeader(), SIGNAL(sectionResized(int,int,int)), this, SLOT(headerStateChangedSlot()));
	connect(ui_.tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(headerStateChangedSlot()));
	connect(ui_.tableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(currentRowChanged(QModelIndex, QModelIndex)));
	connect(&deckTableModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)));
}

DeckWidget::~DeckWidget()
{
}

void DeckWidget::reload()
{
	deckTableModel_.reload();
}

void DeckWidget::save(const QString& filename)
{
	deckTableModel_.save(filename);
}

const QString& DeckWidget::getFilename() const
{
	return deckTableModel_.getFilename();
}

QString DeckWidget::getDisplayName() const
{
	return deckTableModel_.getDisplayName();
}

bool DeckWidget::hasUnsavedChanges() const
{
	return deckTableModel_.hasUnsavedChanges();
}

bool DeckWidget::isDeckActive() const
{
	return deckTableModel_.isDeckActive();
}

void DeckWidget::setDeckActive(const bool active)
{
	deckTableModel_.setDeckActive(active);
	emit deckEdited();
}

void DeckWidget::setHeaderState(const QByteArray& headerState)
{
	if (!headerState.isEmpty())
	{
		ui_.tableView->horizontalHeader()->restoreState(headerState);
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

const DeckTableModel& DeckWidget::getModel() const
{
	return deckTableModel_;
}

void DeckWidget::addToDeck(const QVector<int>& dataRowIndices)
{
	for (const int dataRowIndex : dataRowIndices)
	{
		auto currentQuantity = deckTableModel_.getQuantity(dataRowIndex);
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
		auto currentQuantity = deckTableModel_.getQuantity(dataRowIndex);
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
		ui_.tableView->horizontalHeader()->setSectionHidden(a->data().toInt(), !a->isChecked());
		headerStateChangedSlot();
	}
}

void DeckWidget::headerStateChangedSlot()
{
	emit headerStateChangedSignal(ui_.tableView->horizontalHeader()->saveState());
}
