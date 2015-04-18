#pragma once

#include "filter.h"

#include <QSortFilterProxyModel>

class MagicSortFilterProxyModel : public QSortFilterProxyModel
{
public:

	void setFilterRootNode(FilterNode node)
	{
		filterRootNode_ = std::move(node);
		invalidateFilter();
	}
protected:
	virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
	virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

private:
	FilterNode filterRootNode_;
};
