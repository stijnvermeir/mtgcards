#pragma once

#include <QSortFilterProxyModel>

namespace mtg {

enum PoolColumn
{
	Set,
	Name,
	Names,
	Color,
	Layout,

	ColumnCount
};

}

class PoolTableModel : public virtual QSortFilterProxyModel
{
public:

	PoolTableModel();
	~PoolTableModel();

	void reload();
	QStringList getPictureFilenames(int row);

private:
	class Pimpl;
	Pimpl* pimpl_;
};
