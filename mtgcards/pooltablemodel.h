#pragma once

#include "magicsortfilterproxymodel.h"

#include <QScopedPointer>

class PoolTableModel : public MagicSortFilterProxyModel
{
public:

	PoolTableModel();
	~PoolTableModel();

	virtual int columnToIndex(const mtg::ColumnType& column) const;
	virtual int getDataRowIndex(const QModelIndex& proxyIndex) const;
	virtual mtg::ColumnType columnIndexToType(const int columnIndex) const;

private:
	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
