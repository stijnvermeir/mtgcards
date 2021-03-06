#pragma once

#include "magicsortfilterproxymodel.h"

#include <QScopedPointer>

class CollectionTableModel : public MagicSortFilterProxyModel
{
public:

	CollectionTableModel();
	~CollectionTableModel();

	void reload();

	int getQuantity(const int dataRowIndex) const;
	void setQuantity(const int dataRowIndex, const int newQuantity);

	virtual int columnToIndex(const mtg::ColumnType& column) const;
	virtual int getDataRowIndex(const QModelIndex& proxyIndex) const;
	mtg::ColumnType columnIndexToType(const int columnIndex) const;

	void updateUsedCount();

private:
	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
