#include "poolwindow.h"

#include "magiccarddata.h"
#include "magicitemdelegate.h"
#include "filtereditordialog.h"

#include <QSettings>
#include <QCloseEvent>
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
	ui_.poolTbl_->horizontalHeader()->setSectionsMovable(true);
	ui_.poolTbl_->setSortingEnabled(true);
	ui_.poolTbl_->setSelectionBehavior(QAbstractItemView::SelectRows);

	connect(ui_.poolTbl_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(currentRowChanged(QModelIndex, QModelIndex)));

	connect(ui_.actionAdvancedFilter, SIGNAL(triggered()), this, SLOT(actionAdvancedFilter()));
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

void PoolWindow::currentRowChanged(QModelIndex current, QModelIndex /*previous*/)
{
	auto mappedIdx = poolTableModel_.mapToSource(current);
	auto rv = mtg::CardData::instance().getPictureFilenames(mappedIdx.row());
	emit selectCardChanged(rv.first, rv.second);
}

void PoolWindow::actionAdvancedFilter()
{
	FilterEditorDialog editor;
	editor.setFilterRootNode(rootFilterNode_);
	editor.exec();
	rootFilterNode_ = editor.getFilterRootNode();
	poolTableModel_.setFilterRootNode(rootFilterNode_);
}
