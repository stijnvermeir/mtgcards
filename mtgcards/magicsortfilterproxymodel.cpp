#include "magicsortfilterproxymodel.h"
#include "manacost.h"

#include <functional>

using namespace std;

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
			return node->getFilter().function->apply(data(index(source_row, columnIndex)));
		}
		return false;
	};

	return applyFilter(filterRootNode_);
}
