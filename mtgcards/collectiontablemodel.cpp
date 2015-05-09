#include "collectiontablemodel.h"

#include "magiccollection.h"

#include <QAbstractTableModel>
#include <QDebug>

using namespace std;

namespace {

const vector<mtg::ColumnType> COLLECTIONTABLE_COLUMNS =
{
	mtg::ColumnType::Set,
	mtg::ColumnType::SetCode,
	mtg::ColumnType::SetReleaseDate,
	mtg::ColumnType::SetType,
	mtg::ColumnType::Block,
	mtg::ColumnType::Name,
	mtg::ColumnType::Names,
	mtg::ColumnType::Quantity,
	mtg::ColumnType::Used,
	mtg::ColumnType::ManaCost,
	mtg::ColumnType::CMC,
	mtg::ColumnType::Color,
	mtg::ColumnType::Type,
	mtg::ColumnType::SuperTypes,
	mtg::ColumnType::Types,
	mtg::ColumnType::SubTypes,
	mtg::ColumnType::Rarity,
	mtg::ColumnType::Text,
	mtg::ColumnType::Flavor,
	mtg::ColumnType::Artist,
	mtg::ColumnType::Power,
	mtg::ColumnType::Toughness,
	mtg::ColumnType::Loyalty,
	mtg::ColumnType::Layout,
	mtg::ColumnType::ImageName
};

} // namespace

struct CollectionTableModel::Pimpl : public virtual QAbstractTableModel
{
	Pimpl()
	{
		loadData();
	}

	void loadData()
	{
		beginResetModel();
		mtg::Collection::instance().load();
		endResetModel();
	}

	int getQuantity(const int dataRowIndex) const
	{
		return mtg::Collection::instance().getQuantity(dataRowIndex);
	}

	void setQuantity(const int dataRowIndex, const int newQuantity)
	{
		beginResetModel();
		mtg::Collection::instance().setQuantity(dataRowIndex, newQuantity);
		endResetModel();
	}

	virtual int rowCount(const QModelIndex& = QModelIndex()) const
	{
		return mtg::Collection::instance().getNumRows();
	}

	virtual int columnCount(const QModelIndex& = QModelIndex()) const
	{
		return static_cast<int>(COLLECTIONTABLE_COLUMNS.size());
	}

	virtual QVariant data(const QModelIndex& index, int role) const
	{
		if (index.isValid())
		{
			if (role == Qt::DisplayRole)
			{
				if (index.row() < rowCount() && index.column() < columnCount())
				{
					const QVariant& ret = mtg::Collection::instance().get(index.row(), COLLECTIONTABLE_COLUMNS[index.column()]);
					if (ret.type() == QVariant::StringList)
					{
						return ret.toStringList().join("/");
					}
					return ret;
				}
			}
		}
		return QVariant();
	}

	virtual bool setData(const QModelIndex& index, const QVariant& value, int role)
	{
		if (index.isValid())
		{
			if (role == Qt::EditRole)
			{
				if (index.row() < rowCount() && index.column() < columnCount())
				{
					int dataRowIndex = mtg::Collection::instance().getDataRowIndex(index.row());
					if (COLLECTIONTABLE_COLUMNS[index.column()] == mtg::ColumnType::Quantity)
					{
						if (mtg::Collection::instance().getQuantity(dataRowIndex) != value.toInt() && value.toInt() > 0)
						{
							mtg::Collection::instance().setQuantity(dataRowIndex, value.toInt());
							mtg::Collection::instance().save();
							emit dataChanged(index, index);
							return true;
						}
					}
					else
					if (COLLECTIONTABLE_COLUMNS[index.column()] == mtg::ColumnType::Used)
					{
						mtg::Collection::instance().setUsedCount(dataRowIndex, value.toInt());
						emit dataChanged(index, index);
						return true;
					}
				}
			}
		}
		return false;
	}

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal)
		{
			if (role == Qt::DisplayRole)
			{
				if (section >= 0 && section < columnCount())
				{
					return static_cast<QString>(COLLECTIONTABLE_COLUMNS[section]);
				}
			}
		}
		return QVariant();
	}

	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		if (index.column() < columnCount())
		{
			if (COLLECTIONTABLE_COLUMNS[index.column()] == mtg::ColumnType::Quantity)
			{
				return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
			}
		}
		return QAbstractTableModel::flags(index);
	}
};

CollectionTableModel::CollectionTableModel()
	: pimpl_(new Pimpl())
{
	setSourceModel(pimpl_.data());
}

CollectionTableModel::~CollectionTableModel()
{
}

void CollectionTableModel::reload()
{
	pimpl_->loadData();
}

int CollectionTableModel::getQuantity(const int dataRowIndex) const
{
	return pimpl_->getQuantity(dataRowIndex);
}

void CollectionTableModel::setQuantity(const int dataRowIndex, const int newQuantity)
{
	pimpl_->setQuantity(dataRowIndex, newQuantity);
}

int CollectionTableModel::columnToIndex(const mtg::ColumnType& column) const
{
	auto it = find(COLLECTIONTABLE_COLUMNS.begin(), COLLECTIONTABLE_COLUMNS.end(), column);
	if (it != COLLECTIONTABLE_COLUMNS.end())
	{
		return (it - COLLECTIONTABLE_COLUMNS.begin());
	}
	return -1;
}

mtg::ColumnType CollectionTableModel::columnIndexToType(const int columnIndex) const
{
	if (columnIndex >= 0 && columnIndex < sourceModel()->columnCount())
	{
		return COLLECTIONTABLE_COLUMNS[columnIndex];
	}
	return mtg::ColumnType::UNKNOWN;
}
