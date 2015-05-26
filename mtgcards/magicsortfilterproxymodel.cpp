#include "magicsortfilterproxymodel.h"

#include "manacost.h"
#include "magiccarddata.h"
#include "settings.h"

#include <QDebug>
#include <QUrl>

#include <functional>

using namespace std;

const FilterNode::Ptr& MagicSortFilterProxyModel::getFilterRootNode() const
{
	return filterRootNode_;
}

void MagicSortFilterProxyModel::setFilterRootNode(const FilterNode::Ptr& rootNode)
{
	filterRootNode_ = rootNode;
	invalidateFilter();
}

bool MagicSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
	if (left.data().canConvert<ManaCost>())
	{
		return qvariant_cast<ManaCost>(left.data()) < qvariant_cast<ManaCost>(right.data());
	}
	return QSortFilterProxyModel::lessThan(left, right);
}

bool MagicSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& /*source_parent*/) const
{
	function<bool(const FilterNode::Ptr&)> applyFilter = [this, source_row, &applyFilter](const FilterNode::Ptr& node)
	{
		if (!node)
		{
			return true;
		}

		if (node->getType() == FilterNode::Type::AND)
		{
			for (const FilterNode::Ptr& child : node->getChildren())
			{
				if (!applyFilter(child))
				{
					return false;
				}
			}
			return true;
		}
		else
		if (node->getType() == FilterNode::Type::OR)
		{
			for (const FilterNode::Ptr& child : node->getChildren())
			{
				if (applyFilter(child))
				{
					return true;
				}
			}
			return false;
		}
		else
		if (node->getType() == FilterNode::Type::LEAF)
		{
			auto columnIndex = columnToIndex(node->getFilter().column);
			if (columnIndex < 0)
			{
				return true;
			}
			if (!node->getFilter().function)
			{
				return true;
			}
			QVariant d = sourceModel()->data(sourceModel()->index(source_row, columnIndex));
			if (node->getFilter().negate)
			{
				return !(node->getFilter().function->apply(d));
			}
			return node->getFilter().function->apply(d);
		}
		return false;
	};

	FilterNode::Ptr local = filterRootNode_;
	FilterNode::Ptr global = Settings::instance().getGlobalFilter();
	FilterNode::Ptr node = FilterNode::create();
	node->setType(FilterNode::Type::AND);
	node->addChild(local);
	node->addChild(global);
	bool rv = applyFilter(node);
	node->removeChild(global);
	node->removeChild(local);
	return rv;
}

QVariant MagicSortFilterProxyModel::data(const QModelIndex& index, int role) const
{
	QVariant d = QSortFilterProxyModel::data(index, role);
	if (d.canConvert<ManaCost>() && role == Qt::ToolTipRole)
	{
		return qvariant_cast<ManaCost>(d).getText();
	}
	if (index.column() == columnToIndex(mtg::ColumnType::Name) && role == Qt::ToolTipRole)
	{
		auto picInfo = mtg::CardData::instance().getPictureInfo(getDataRowIndex(index));
		if (picInfo.missing.empty())
		{
			return QString("<img src=\"") + picInfo.filenames.front() + "\" />";
		}
	}
	return d;
}
