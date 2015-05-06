#include "pooltablemodel.h"
#include "magiccarddata.h"

#include <QAbstractTableModel>
#include <QDebug>

using namespace std;

namespace {

const vector<mtg::ColumnType> POOLTABLE_COLUMNS =
{
	mtg::ColumnType::Set,
	mtg::ColumnType::SetCode,
	mtg::ColumnType::SetReleaseDate,
	mtg::ColumnType::SetType,
	mtg::ColumnType::Block,
	mtg::ColumnType::Name,
	mtg::ColumnType::Names,
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

class PoolTableModel::Pimpl : public virtual QAbstractTableModel
{
public:

	Pimpl()
	{
		loadData();
	}

	void loadData()
	{
		beginResetModel();
		mtg::CardData::instance().reload();
		endResetModel();
	}

	virtual int rowCount(const QModelIndex& ) const
	{
		return mtg::CardData::instance().getNumRows();
	}

	virtual int columnCount(const QModelIndex& = QModelIndex()) const
	{
		return static_cast<int>(POOLTABLE_COLUMNS.size());
	}

	virtual QVariant data(const QModelIndex& index, int role) const
	{
		if (index.isValid())
		{
			if (role == Qt::DisplayRole)
			{
				if (index.row() < mtg::CardData::instance().getNumRows() && index.column() < columnCount())
				{
					const QVariant& ret = mtg::CardData::instance().get(index.row(), POOLTABLE_COLUMNS[index.column()]);
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
					return static_cast<QString>(POOLTABLE_COLUMNS[section]);
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

PoolTableModel::PoolTableModel()
	: pimpl_(new Pimpl())
{
	setSourceModel(pimpl_.get());
}

PoolTableModel::~PoolTableModel()
{
}

void PoolTableModel::reload()
{
	pimpl_->loadData();
}

int PoolTableModel::columnToIndex(const mtg::ColumnType& column) const
{
	auto it = find(POOLTABLE_COLUMNS.begin(), POOLTABLE_COLUMNS.end(), column);
	if (it != POOLTABLE_COLUMNS.end())
	{
		return (it - POOLTABLE_COLUMNS.begin());
	}
	return -1;
}
