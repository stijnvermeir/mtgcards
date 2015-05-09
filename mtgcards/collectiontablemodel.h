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
	mtg::ColumnType columnIndexToType(const int columnIndex) const;

private:
	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
