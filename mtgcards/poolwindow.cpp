#include "poolwindow.h"

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

const QString DEFAULT_HEADER_STATE = "{\"sections\":[{\"hidden\":true,\"size\":187,\"visualIndex\":0},{\"hidden\":false,\"size\":44,\"visualIndex\":2},{\"hidden\":true,\"size\":100,\"visualIndex\":3},{\"hidden\":true,\"size\":100,\"visualIndex\":4},{\"hidden\":false,\"size\":119,\"visualIndex\":1},{\"hidden\":true,\"size\":122,\"visualIndex\":5},{\"hidden\":true,\"size\":100,\"visualIndex\":6},{\"hidden\":true,\"size\":97,\"visualIndex\":7},{\"hidden\":true,\"size\":81,\"visualIndex\":8},{\"hidden\":false,\"size\":247,\"visualIndex\":9},{\"hidden\":true,\"size\":314,\"visualIndex\":10},{\"hidden\":false,\"size\":219,\"visualIndex\":12},{\"hidden\":false,\"size\":36,\"visualIndex\":13},{\"hidden\":false,\"size\":100,\"visualIndex\":14},{\"hidden\":false,\"size\":272,\"visualIndex\":15},{\"hidden\":true,\"size\":83,\"visualIndex\":16},{\"hidden\":true,\"size\":100,\"visualIndex\":17},{\"hidden\":true,\"size\":100,\"visualIndex\":18},{\"hidden\":false,\"size\":112,\"visualIndex\":19},{\"hidden\":true,\"size\":53,\"visualIndex\":20},{\"hidden\":true,\"size\":100,\"visualIndex\":21},{\"hidden\":true,\"size\":170,\"visualIndex\":24},{\"hidden\":false,\"size\":31,\"visualIndex\":22},{\"hidden\":false,\"size\":29,\"visualIndex\":23},{\"hidden\":true,\"size\":100,\"visualIndex\":25},{\"hidden\":true,\"size\":100,\"visualIndex\":26},{\"hidden\":true,\"size\":100,\"visualIndex\":11},{\"hidden\":true,\"size\":100,\"visualIndex\":27},{\"hidden\":false,\"size\":109,\"visualIndex\":28},{\"hidden\":true,\"size\":100,\"visualIndex\":29},{\"hidden\":true,\"size\":82,\"visualIndex\":30},{\"hidden\":false,\"size\":86,\"visualIndex\":31},{\"hidden\":true,\"size\":44,\"visualIndex\":32}],\"sortIndicatorOrder\":1,\"sortIndicatorSection\":4}";

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

PoolWindow::PoolWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, poolTableModel_()
	, itemDelegate_(new PoolItemDelegate(poolTableModel_))
	, rootFilterNode_()
{
	setWindowFlags(Qt::NoDropShadowWindowHint);
	ui_.setupUi(this);
	ui_.poolTbl_->setItemDelegate(itemDelegate_.data());
	ui_.poolTbl_->setModel(&poolTableModel_);
	ui_.poolTbl_->setSortingEnabled(true);
	ui_.poolTbl_->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui_.poolTbl_->horizontalHeader()->setSectionsMovable(true);
	ui_.poolTbl_->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui_.poolTbl_->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(hideColumnsContextMenuRequested(QPoint)));
	connect(ui_.poolTbl_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(currentRowChanged(QModelIndex, QModelIndex)));
	connect(ui_.actionAdvancedFilter, SIGNAL(triggered()), this, SLOT(actionAdvancedFilter()));
	connect(ui_.actionEnableFilter, SIGNAL(triggered(bool)), this, SLOT(actionEnableFilter(bool)));
	connect(ui_.actionAddToCollection, SIGNAL(triggered()), this, SLOT(actionAddToCollection()));
	connect(ui_.actionRemoveFromCollection, SIGNAL(triggered()), this, SLOT(actionRemoveFromCollection()));
	connect(ui_.actionAddToDeck, SIGNAL(triggered()), this, SLOT(actionAddToDeck()));
	connect(ui_.actionRemoveFromDeck, SIGNAL(triggered()), this, SLOT(actionRemoveFromDeck()));
	connect(ui_.actionDownloadCardArt, SIGNAL(triggered()), this, SLOT(actionDownloadCardArt()));
	connect(ui_.actionFetchOnlineData, SIGNAL(triggered()), this, SLOT(actionFetchOnlineData()));
	connect(this, SIGNAL(fontChanged()), ui_.poolTbl_, SLOT(handleFontChanged()));

	ui_.statusBar->addPermanentWidget(new QLabel("Search: "));
	QLabel* permanentStatusBarLabel = new QLabel();
	ui_.statusBar->addPermanentWidget(permanentStatusBarLabel);
	connect(ui_.poolTbl_, SIGNAL(searchStringChanged(QString)), permanentStatusBarLabel, SLOT(setText(QString)));
}

PoolWindow::~PoolWindow()
{
}

void PoolWindow::updateShortcuts()
{
	ui_.actionAdvancedFilter->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AdvancedFilter]);
	ui_.actionAddToCollection->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AddToCollection]);
	ui_.actionRemoveFromCollection->setShortcut(Settings::instance().getShortcuts()[ShortcutType::RemoveFromCollection]);
	ui_.actionAddToDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AddToDeck]);
	ui_.actionRemoveFromDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::RemoveFromDeck]);
}

void PoolWindow::loadSettings()
{
	QSettings settings;
	Util::loadHeaderViewState(*ui_.poolTbl_->horizontalHeader(), settings.value("poolwindow/headerstate", DEFAULT_HEADER_STATE).toString());
	if (settings.contains("poolwindow/filterEnable"))
	{
		ui_.actionEnableFilter->setChecked(settings.value("poolwindow/filterEnable").toBool());
	}
	if (settings.contains("poolwindow/filter"))
	{
		rootFilterNode_ = FilterNode::createFromJson(QJsonDocument::fromJson(settings.value("poolwindow/filter").toString().toUtf8()));
		if (ui_.actionEnableFilter->isChecked())
		{
			poolTableModel_.setFilterRootNode(rootFilterNode_);
		}
	}
	updateStatusBar();
	updateShortcuts();
}

void PoolWindow::saveSettings()
{
	QSettings settings;
	settings.setValue("poolwindow/headerstate", Util::saveHeaderViewState(*ui_.poolTbl_->horizontalHeader()));
	settings.setValue("poolwindow/filterEnable", ui_.actionEnableFilter->isChecked());
	if (rootFilterNode_)
	{
		settings.setValue("poolwindow/filter", QString(rootFilterNode_->toJson().toJson(QJsonDocument::Compact)));
	}
	else
	{
		settings.remove("poolwindow/filter");
	}
}

void PoolWindow::closeEvent(QCloseEvent* event)
{
	emit windowClosed(false);
	event->accept();
}

bool PoolWindow::event(QEvent* event)
{
	if (event->type() == QEvent::WindowActivate)
	{
		emit selectedCardChanged(currentDataRowIndex());
	}
	return QMainWindow::event(event);
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

void PoolWindow::updateStatusBar()
{
	ui_.statusBar->showMessage(QString::number(poolTableModel_.rowCount()) + " cards");
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
	if (ui_.actionEnableFilter->isChecked())
	{
		poolTableModel_.setFilterRootNode(rootFilterNode_);
	}
	updateStatusBar();
}

void PoolWindow::actionEnableFilter(bool enable)
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

void PoolWindow::actionDownloadCardArt()
{
	poolTableModel_.downloadCardArt(ui_.poolTbl_->selectionModel()->selectedRows());
}

void PoolWindow::actionFetchOnlineData()
{
	poolTableModel_.fetchOnlineData(ui_.poolTbl_->selectionModel()->selectedRows());
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

void PoolWindow::handleGlobalFilterChanged()
{
	if (ui_.actionEnableFilter->isChecked())
	{
		poolTableModel_.setFilterRootNode(rootFilterNode_);
	}
	else
	{
		poolTableModel_.setFilterRootNode(FilterNode::Ptr());
	}
	updateStatusBar();
}
