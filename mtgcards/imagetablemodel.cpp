#include "imagetablemodel.h"

#include "magiccarddata.h"

#include <QPixmap>
#include <QDebug>

ImageTableModel::ImageTableModel(MagicSortFilterProxyModel* proxy, QObject *parent)
    : QAbstractTableModel(parent)
    , numColumns_(8)
    , imageSize_(240, 340)
    , proxy_(proxy)
{
}

int ImageTableModel::rowCount(const QModelIndex&) const
{
	return proxy_->rowCount() / numColumns_ + ((proxy_->rowCount() % numColumns_) ? 1 : 0);
}

int ImageTableModel::columnCount(const QModelIndex&) const
{
	return numColumns_;
}

QVariant ImageTableModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	int proxyRow = index.row() * numColumns_ + index.column();
	if (proxyRow >= proxy_->rowCount())
	{
		return QVariant();
	}

	QModelIndex proxyIndex = proxy_->index(proxyRow, 0);
	int dataRowIndex = proxy_->getDataRowIndex(proxyIndex);
	if (dataRowIndex < mtg::CardData::instance().getNumRows())
	{
		if (role == Qt::DecorationRole)
		{
			auto picInfo = mtg::CardData::instance().getPictureInfo(dataRowIndex);
			return QPixmap(picInfo.filenames.front()).scaled(imageSize_.width(), imageSize_.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}
	}
	return QVariant();
}

void ImageTableModel::setNumColumns(int numColumns)
{
	if (numColumns < 1)
	{
		numColumns = 1;
	}
	beginResetModel();
	numColumns_ = numColumns;
	endResetModel();
}

void ImageTableModel::setImageSize(const QSize& imageSize)
{
	imageSize_ = imageSize;
}

void ImageTableModel::reset()
{
	beginResetModel();
	endResetModel();
}
