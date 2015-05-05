#pragma once

#include "magicsortfilterproxymodel.h"

#include <memory>

class CollectionTableModel : public MagicSortFilterProxyModel
{
public:

	CollectionTableModel();
	~CollectionTableModel();

	void reload();
	void addCard(int row);

private:
	virtual int columnToIndex(const mtg::ColumnType& column) const;

	class Pimpl;
	std::unique_ptr<Pimpl> pimpl_;
};
