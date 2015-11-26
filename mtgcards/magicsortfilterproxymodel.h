#pragma once

#include "filter.h"

#include <QSortFilterProxyModel>

class MagicSortFilterProxyModel : public QSortFilterProxyModel
{
public:

	const FilterNode::Ptr& getFilterRootNode() const;
	void setFilterRootNode(const FilterNode::Ptr& node);

	void fetchOnlineData(const QModelIndexList& selectedRows);

	virtual int columnToIndex(const mtg::ColumnType& column) const = 0;
	virtual int getDataRowIndex(const QModelIndex& proxyIndex) const = 0;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	virtual Qt::ItemFlags flags(const QModelIndex& index) const;

protected:
	virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
	virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

private:
	FilterNode::Ptr filterRootNode_;
};
