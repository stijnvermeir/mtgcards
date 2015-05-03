#pragma once

#include "magicsortfilterproxymodel.h"

#include <memory>

class PoolTableModel : public MagicSortFilterProxyModel
{
public:

	PoolTableModel();
	~PoolTableModel();

	void reload();

private:
	virtual int columnToIndex(const mtg::ColumnType& column) const;

	class Pimpl;
	std::unique_ptr<Pimpl> pimpl_;
};
