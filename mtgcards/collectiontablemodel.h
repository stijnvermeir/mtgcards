#pragma once

#include "magicsortfilterproxymodel.h"

#include <memory>

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

	class Pimpl;
	std::unique_ptr<Pimpl> pimpl_;
};
