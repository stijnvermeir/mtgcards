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
{
	ui_.setupUi(this);

	ui_.poolTbl_->setItemDelegate(new MagicItemDelegate());
	ui_.poolTbl_->setModel(&poolTableModel_);
	ui_.poolTbl_->horizontalHeader()->setSectionsMovable(true);
	ui_.poolTbl_->setSortingEnabled(true);
	ui_.poolTbl_->setSelectionBehavior(QAbstractItemView::SelectRows);

	connect(ui_.poolTbl_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(currentRowChanged(QModelIndex, QModelIndex)));

	connect(ui_.actionAdvancedFilter, SIGNAL(toggled(bool)), this, SLOT(actionAdvancedFilterToggled(bool)));
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

void PoolWindow::actionAdvancedFilterToggled(bool enabled)
{
	if (enabled)
	{
		FilterEditorDialog editor;
		FilterNode::Ptr root = FilterNode::create();
		/*
		root->setType(FilterNode::Type::AND);
		FilterNode::Ptr a = FilterNode::create();
		root->addChild(a);
		a->setType(FilterNode::Type::OR);
		FilterNode::Ptr a1 = FilterNode::create();
		a->addChild(a1);
		a1->setType(FilterNode::Type::LEAF);
		Filter a1Filter;
		a1Filter.column = mtg::ColumnType::ManaCost;
		a1Filter.function = FilterFunctionFactory::createRegex(".*R.*");
		a1->setFilter(std::move(a1Filter));
		FilterNode::Ptr a2 = FilterNode::create();
		a->addChild(a2);
		a2->setType(FilterNode::Type::LEAF);
		Filter a2Filter;
		a2Filter.column = mtg::ColumnType::Text;
		a2Filter.function = FilterFunctionFactory::createRegex(".*counter.*");
		a2->setFilter(std::move(a2Filter));
		FilterNode::Ptr b = FilterNode::create();
		root->addChild(b);
		b->setType(FilterNode::Type::LEAF);
		Filter bFilter;
		bFilter.column = mtg::ColumnType::SetCode;
		bFilter.function = FilterFunctionFactory::createRegex("M15");
		b->setFilter(std::move(bFilter));
		root->saveToFile("test.json");
		*/
		root->loadFromFile("test.json");

		editor.setFilterRootNode(root);

		//editor.setFilterRootNode(poolTableModel_.getFilterRootNode());
		editor.exec();
		//poolTableModel_.setFilterRootNode(editor.getFilterRootNode());
	}
	else
	{
		poolTableModel_.setFilterRootNode(FilterNode::Ptr());
	}
}
