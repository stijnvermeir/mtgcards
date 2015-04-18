#include "poolwindow.h"
#include "manacostdelegate.h"

#include <QSettings>
#include <QCloseEvent>
#include <QDebug>

PoolWindow::PoolWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, poolTableModel_()
{
	ui_.setupUi(this);

	ui_.poolTbl_->setItemDelegate(new ManaCostDelegate());
	ui_.poolTbl_->setModel(&poolTableModel_);
	ui_.poolTbl_->horizontalHeader()->setSectionsMovable(true);
	ui_.poolTbl_->setSortingEnabled(true);
	ui_.poolTbl_->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui_.poolTbl_->resizeColumnsToContents();
	poolTableModel_.sort(-1);

	connect(ui_.poolTbl_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(currentRowChanged(QModelIndex, QModelIndex)));

#if 0 // test card picture availability
	for (int i = 0; i < poolTableModel_.rowCount(); ++i)
	{
		poolTableModel_.getPictureFilenames(i);
	}
#endif

#if 0 // test filter tree
	FilterNode root;
	root.type = FilterNode::Type::AND;
	root.children.push_back(FilterNode::create());
	root.children.back()->type = FilterNode::Type::LEAF;
	root.children.back()->filter.column = mtg::Text;
	root.children.back()->filter.function = [](QVariant data)
	{
		return data.toString().contains("+1/+1");
	};
	root.children.push_back(FilterNode::create());
	root.children.back()->type = FilterNode::Type::OR;
	root.children.back()->children.push_back(FilterNode::create());
	root.children.back()->children.back()->type = FilterNode::Type::LEAF;
	root.children.back()->children.back()->filter.column = mtg::CMC;
	root.children.back()->children.back()->filter.function = [](QVariant data)
	{
		return data.toInt() == 3;
	};
	root.children.back()->children.push_back(FilterNode::create());
	root.children.back()->children.back()->type = FilterNode::Type::LEAF;
	root.children.back()->children.back()->filter.column = mtg::Rarity;
	root.children.back()->children.back()->filter.function = [](QVariant data)
	{
		return data.toString() == "Mythic Rare";
	};
	poolTableModel_.setFilterRootNode(std::move(root));
#endif
}

PoolWindow::~PoolWindow()
{
}

void PoolWindow::reload()
{
	poolTableModel_.reload();
	ui_.poolTbl_->resizeColumnsToContents();
}

void PoolWindow::closeEvent(QCloseEvent* event)
{
	emit windowClosed(false);
	event->accept();
}

void PoolWindow::currentRowChanged(QModelIndex current, QModelIndex /*previous*/)
{
	auto mappedIdx = poolTableModel_.mapToSource(current);
	auto rv = poolTableModel_.getPictureFilenames(mappedIdx.row());
	emit selectCardChanged(rv.first, rv.second);
}
