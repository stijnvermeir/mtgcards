#pragma once

#include <QSortFilterProxyModel>

class MagicSortFilterProxyModel : public QSortFilterProxyModel
{
protected:
	virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
};
