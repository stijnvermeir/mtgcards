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

PoolWindow::PoolWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, poolTableModel_()
	, rootFilterNode_()
{
	setWindowFlags(Qt::NoDropShadowWindowHint);
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

	ui_.statusBar->addPermanentWidget(new QLabel("Search: "));
	QLabel* permanentStatusBarLabel = new QLabel();
	ui_.statusBar->addPermanentWidget(permanentStatusBarLabel);
	connect(ui_.poolTbl_, SIGNAL(searchStringChanged(QString)), permanentStatusBarLabel, SLOT(setText(QString)));
}

PoolWindow::~PoolWindow()
{
}

void PoolWindow::reload()
{
	poolTableModel_.reload();
	updateStatusBar();
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
	if (settings.contains("poolwindow/headerstate"))
	{
		Util::loadHeaderViewState(*ui_.poolTbl_->horizontalHeader(), settings.value("poolwindow/headerstate").toString());
	}
	else
	{
		ui_.poolTbl_->resizeColumnsToContents();
	}
	if (settings.contains("poolwindow/filter"))
	{
		rootFilterNode_ = FilterNode::createFromJson(QJsonDocument::fromJson(settings.value("poolwindow/filter").toString().toUtf8()));
		poolTableModel_.setFilterRootNode(rootFilterNode_);
	}
	updateStatusBar();
	updateShortcuts();
}

void PoolWindow::saveSettings()
{
	QSettings settings;
	settings.setValue("poolwindow/headerstate", Util::saveHeaderViewState(*ui_.poolTbl_->horizontalHeader()));
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
	if (event->type() == QEvent::WindowActivate /*|| event->type() == QEvent::Enter*/)
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
	poolTableModel_.setFilterRootNode(rootFilterNode_);
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
