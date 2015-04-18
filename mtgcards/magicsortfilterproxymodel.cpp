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
	function<bool(const FilterNode&)> applyFilter = [this, source_row, &applyFilter](const FilterNode& node)
	{
		if (node.type == FilterNode::Type::AND)
		{
			for (const FilterNode::Ptr& child : node.children)
			{
				if (!applyFilter(*child))
				{
					return false;
				}
			}
			return true;
		}
		else
		if (node.type == FilterNode::Type::OR)
		{
			for (const FilterNode::Ptr& child : node.children)
			{
				if (applyFilter(*child))
				{
					return true;
				}
			}
			return false;
		}
		else
		if (node.type == FilterNode::Type::LEAF)
		{
			return node.filter.function(data(index(source_row, node.filter.column)));
		}
		return false;
	};

	return applyFilter(filterRootNode_);
}
