#pragma once

#include "enum.h"
#include "magicsortfilterproxymodel.h"

namespace mtg {

enum PoolColumn
{
	// visible columns
	Set,
	SetCode,
	SetReleaseDate,
	SetType,
	Block,
	Name,
	Names,
	ManaCost,
	CMC,
	Color,
	Type,
	SuperTypes,
	Types,
	SubTypes,
	Rarity,
	Text,
	Flavor,
	Artist,
	Power,
	Toughness,
	Loyalty,

	VisibleColumnCount,

	// hidden columns
	Layout = VisibleColumnCount,
	ImageName,

	TotalColumnCount
};

}

class PoolTableModel : public MagicSortFilterProxyModel
{
public:

	PoolTableModel();
	~PoolTableModel();

	void reload();
	std::pair<layout_type_t, QStringList> getPictureFilenames(int row);

private:
	class Pimpl;
	Pimpl* pimpl_;
};
