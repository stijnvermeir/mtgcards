#pragma once

#include "filter.h"

#include <QSortFilterProxyModel>

class MagicSortFilterProxyModel : public QSortFilterProxyModel
{
public:

	const FilterNode::Ptr& getFilterRootNode() const;
	void setFilterRootNode(const FilterNode::Ptr& node);

	virtual int columnToIndex(const mtg::ColumnType& column) const = 0;

protected:
	virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
	virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

private:
	FilterNode::Ptr filterRootNode_;
};
