#pragma once

#include "magicsortfilterproxymodel.h"

#include <QAbstractTableModel>

class ImageTableModel : public QAbstractTableModel
{
public:
	ImageTableModel(MagicSortFilterProxyModel* proxy, QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	void setNumColumns(int numColumns);
	void setImageSize(const QSize& imageSize);

	void reset();

private:
	int numColumns_;
	QSize imageSize_;
	MagicSortFilterProxyModel* proxy_;
};
