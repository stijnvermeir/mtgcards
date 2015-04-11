#pragma once

#include <QSortFilterProxyModel>

namespace mtg {

enum PoolColumn
{
	Set,
	Name,
	Color,

	ColumnCount
};

}

class PoolTableModel : public virtual QSortFilterProxyModel
{
public:

	PoolTableModel();
	~PoolTableModel();

private:
	class Pimpl;
	Pimpl* pimpl_;
};
