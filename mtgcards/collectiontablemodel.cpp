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
	mtg::ColumnType::Loyalty
};

} // namespace

class CollectionTableModel::Pimpl : public virtual QAbstractTableModel
{
public:
	Pimpl()
	{
		loadData();
	}

	void loadData()
	{
		beginResetModel();
		mtg::Collection::instance().reload();
		endResetModel();
	}

	void addCard(int row)
	{
		beginResetModel();
		int currentQuantity = mtg::Collection::instance().getQuantity(row);
		mtg::Collection::instance().setQuantity(row, currentQuantity + 1);
		endResetModel();
	}

	virtual int rowCount(const QModelIndex& ) const
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
				if (index.row() < mtg::Collection::instance().getNumRows() && index.column() < columnCount())
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
		return QAbstractTableModel::flags(index);
	}
};

CollectionTableModel::CollectionTableModel()
	: pimpl_(new Pimpl())
{
	setSourceModel(pimpl_.get());
}

CollectionTableModel::~CollectionTableModel()
{
}

void CollectionTableModel::reload()
{
	pimpl_->loadData();
}

void CollectionTableModel::addCard(int row)
{
	pimpl_->addCard(row);
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
