#pragma once

#include "magicsortfilterproxymodel.h"

#include <QScopedPointer>

class PoolTableModel : public MagicSortFilterProxyModel
{
public:

	PoolTableModel();
	~PoolTableModel();

	void reload();

private:
	virtual int columnToIndex(const mtg::ColumnType& column) const;

	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
