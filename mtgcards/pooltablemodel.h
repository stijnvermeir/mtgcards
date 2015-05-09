#pragma once

#include "magicsortfilterproxymodel.h"

#include <QScopedPointer>

class PoolTableModel : public MagicSortFilterProxyModel
{
public:

	PoolTableModel();
	~PoolTableModel();

	void reload();

	virtual int columnToIndex(const mtg::ColumnType& column) const;

private:
	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
