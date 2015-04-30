#pragma once

#include "magiclayouttype.h"
#include "magicsortfilterproxymodel.h"

#include <memory>

class PoolTableModel : public MagicSortFilterProxyModel
{
public:

	PoolTableModel();
	~PoolTableModel();

	void reload();
	std::pair<mtg::LayoutType, QStringList> getPictureFilenames(int row);

private:
	class Pimpl;
	std::unique_ptr<Pimpl> pimpl_;
};
