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

private:
	virtual int columnToIndex(const mtg::ColumnType& column) const;

	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
