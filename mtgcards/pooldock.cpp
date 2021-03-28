#include "pooldock.h"

#include "magicitemdelegate.h"
#include "filtereditordialog.h"
#include "settings.h"
#include "util.h"

#include <QSettings>
#include <QCloseEvent>
#include <QMenu>
#include <QLabel>
#include <QDebug>

namespace {

class PoolItemDelegate : public MagicItemDelegate
{
public:
	PoolItemDelegate(const PoolTableModel& model)
		: model_(model)
	{
	}

	virtual mtg::ColumnType columnIndexToType(const int columnIndex) const
	{
		return model_.columnIndexToType(columnIndex);
	}
private:
	const PoolTableModel& model_;
};

}

PoolDock::PoolDock(Ui::MainWindow& ui, QObject* parent)
    : QObject(parent)
    , ui_(ui)
	, poolTableModel_()
    , imageTableModel_(&poolTableModel_)
	, itemDelegate_(new PoolItemDelegate(poolTableModel_))
	, rootFilterNode_()
    , commonActions_(this)
{
	ui_.poolTableView->setItemDelegate(itemDelegate_.data());
	ui_.poolTableView->setModel(&poolTableModel_);
	ui_.poolTableView->setSortingEnabled(true);
	ui_.poolTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui_.poolTableView->horizontalHeader()->setSectionsMovable(true);
	ui_.poolTableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	ui_.poolTableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui_.poolTableView->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(hideColumnsContextMenuRequested(QPoint)));
	connect(ui_.poolTableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(currentRowChanged(QModelIndex, QModelIndex)));
	connect(ui_.poolTableView, &TableView::customContextMenuRequested, this, &PoolDock::rowContextMenuRequested);
	connect(this, SIGNAL(fontChanged()), ui_.poolTableView, SLOT(handleFontChanged()));
	connect(ui_.poolTableView, SIGNAL(searchStringChanged(QString)), ui_.poolStatusBar, SLOT(setSearch(QString)));
	commonActions_.connectSignals(this);
	commonActions_.addToWidget(ui_.poolTableView);
	commonActions_.addToWidget(ui_.poolImageTable);

	ui_.poolImageTable->setImageTableModel(&imageTableModel_);
	connect(ui_.poolImageTable->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(imageCurrentChanged(const QModelIndex&, const QModelIndex&)));

	connect(ui_.poolStatusBar, SIGNAL(viewChanged(int)), this, SLOT(statusBarViewChanged(int)));
	connect(ui_.poolStatusBar, SIGNAL(sliderValueChanged(int)), ui_.poolImageTable, SLOT(changeImageScale(int)));
}

PoolDock::~PoolDock()
{
}

void PoolDock::updateOptions()
{
	commonActions_.updateShortcuts();
	ui_.poolStatusBar->setViewChangerEnabled(Settings::instance().getPoolViewButtons());
}

void PoolDock::loadSettings()
{
	QSettings settings;
	if (settings.contains("poolwindow/headerstate"))
	{
		Util::loadHeaderViewState(*ui_.poolTableView->horizontalHeader(), settings.value("poolwindow/headerstate").toString());
	}
	if (settings.contains("poolwindow/filterEnable"))
	{
		commonActions_.getEnableFilter()->setChecked(settings.value("poolwindow/filterEnable").toBool());
	}
	if (settings.contains("poolwindow/filter"))
	{
		rootFilterNode_ = FilterNode::createFromJson(QJsonDocument::fromJson(settings.value("poolwindow/filter").toString().toUtf8()));
		if (commonActions_.getEnableFilter()->isChecked())
		{
			poolTableModel_.setFilterRootNode(rootFilterNode_);
		}
	}

	ui_.poolStatusBar->setViewIndex(settings.value("poolwindow/viewIndex", 0).toInt());
	ui_.poolStatusBar->setSliderValue(settings.value("poolwindow/imagePromille", 500).toInt());

	updateStatusBar();
	updateOptions();
}

void PoolDock::saveSettings()
{
	QSettings settings;
	settings.setValue("poolwindow/headerstate", Util::saveHeaderViewState(*ui_.poolTableView->horizontalHeader()));
	settings.setValue("poolwindow/filterEnable", commonActions_.getEnableFilter()->isChecked());
	if (rootFilterNode_)
	{
		settings.setValue("poolwindow/filter", QString(rootFilterNode_->toJson().toJson(QJsonDocument::Compact)));
	}
	else
	{
		settings.remove("poolwindow/filter");
	}

	settings.setValue("poolwindow/viewIndex", ui_.poolStatusBar->getViewIndex());
	settings.setValue("poolwindow/imagePromille", ui_.poolStatusBar->getSliderValue());
}

int PoolDock::currentDataRowIndex() const
{
	QModelIndex proxyIndex = ui_.poolTableView->currentIndex();
	QModelIndex sourceIndex = poolTableModel_.mapToSource(proxyIndex);
	return sourceIndex.row();
}

QVector<int> PoolDock::currentDataRowIndices() const
{
	QModelIndexList list = ui_.poolTableView->selectionModel()->selectedRows();
	QVector<int> indices;
	indices.reserve(list.size());
	for (const auto& proxyIndex : list)
	{
		QModelIndex sourceIndex = poolTableModel_.mapToSource(proxyIndex);
		indices.push_back(sourceIndex.row());
	}
	return indices;
}

void PoolDock::updateStatusBar()
{
	ui_.poolStatusBar->setMessage(QString::number(poolTableModel_.rowCount()) + " cards");
}

void PoolDock::currentRowChanged(QModelIndex, QModelIndex)
{
	emit selectedCardChanged(currentDataRowIndex());
}

void PoolDock::actionAdvancedFilter()
{
	FilterEditorDialog editor;
	editor.setFilterRootNode(rootFilterNode_);
	editor.exec();
	rootFilterNode_ = editor.getFilterRootNode();
	if (commonActions_.getEnableFilter()->isChecked())
	{
		poolTableModel_.setFilterRootNode(rootFilterNode_);
	}
	updateStatusBar();
}

void PoolDock::actionEnableFilter(bool enable)
{
	if (enable)
	{
		poolTableModel_.setFilterRootNode(rootFilterNode_);
	}
	else
	{
		poolTableModel_.setFilterRootNode(FilterNode::Ptr());
	}
	updateStatusBar();
}

void PoolDock::actionAddToCollection()
{
	emit addToCollection(currentDataRowIndices());
}

void PoolDock::actionRemoveFromCollection()
{
	emit removeFromCollection(currentDataRowIndices());
}

void PoolDock::actionAddToDeck()
{
	emit addToDeck(currentDataRowIndices());
}

void PoolDock::actionRemoveFromDeck()
{
	emit removeFromDeck(currentDataRowIndices());
}

void PoolDock::actionDownloadCardArt()
{
	poolTableModel_.downloadCardArt(ui_.poolTableView->selectionModel()->selectedRows());
}

void PoolDock::actionFetchOnlineData()
{
	poolTableModel_.fetchOnlineData(ui_.poolTableView->selectionModel()->selectedRows());
}

void PoolDock::hideColumnsContextMenuRequested(const QPoint& pos)
{
	QMenu contextMenu(ui_.poolTableView);
	for (int i = 0; i < poolTableModel_.columnCount(); ++i)
	{
		QAction* action = new QAction(&contextMenu);
		action->setCheckable(true);
		action->setText(poolTableModel_.headerData(i, Qt::Horizontal).toString());
		action->setData(i);
		action->setChecked(!ui_.poolTableView->horizontalHeader()->isSectionHidden(i));
		contextMenu.addAction(action);
	}
	QAction* a = contextMenu.exec(ui_.poolTableView->horizontalHeader()->mapToGlobal(pos));
	if (a)
	{
		ui_.poolTableView->horizontalHeader()->setSectionHidden(a->data().toInt(), !a->isChecked());
	}
}

void PoolDock::rowContextMenuRequested(const QPoint& pos)
{
	QMenu contextMenu(ui_.poolTableView);
	commonActions_.addToMenu(&contextMenu);
	contextMenu.exec(ui_.poolTableView->mapToGlobal(pos));
}

void PoolDock::handleGlobalFilterChanged()
{
	if (commonActions_.getEnableFilter()->isChecked())
	{
		poolTableModel_.setFilterRootNode(rootFilterNode_);
	}
	else
	{
		poolTableModel_.setFilterRootNode(FilterNode::Ptr());
	}
	updateStatusBar();
}

void PoolDock::statusBarViewChanged(int index)
{
	if (index == 0)
	{
		ui_.poolStatusBar->setSearchEnabled(true);
		ui_.poolStatusBar->setSliderEnabled(false);
	}
	if (index == 1)
	{
		ui_.poolStatusBar->setSearchEnabled(false);
		ui_.poolStatusBar->setSliderEnabled(true);
		imageTableModel_.reset();
		int currentRow = ui_.poolTableView->currentIndex().row();
		int imageColumnCount = imageTableModel_.columnCount();
		QModelIndex indexToSelect = imageTableModel_.index(currentRow / imageColumnCount, currentRow % imageColumnCount);
		ui_.poolImageTable->selectionModel()->select(indexToSelect, QItemSelectionModel::Select);
		ui_.poolImageTable->scrollTo(indexToSelect);
	}
	ui_.poolStack->setCurrentIndex(index);
}

void PoolDock::imageCurrentChanged(const QModelIndex& current, const QModelIndex&)
{
	ui_.poolTableView->selectRow(current.row() * imageTableModel_.columnCount() + current.column());
}
