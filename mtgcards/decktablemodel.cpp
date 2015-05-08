#include "decktablemodel.h"
#include "deck.h"

#include <QAbstractTableModel>
#include <QDebug>

using namespace std;

namespace {

const vector<mtg::ColumnType> DECKTABLE_COLUMNS =
{
	mtg::ColumnType::Set,
	mtg::ColumnType::SetCode,
	mtg::ColumnType::SetReleaseDate,
	mtg::ColumnType::SetType,
	mtg::ColumnType::Block,
	mtg::ColumnType::Name,
	mtg::ColumnType::Names,
	mtg::ColumnType::Quantity,
	mtg::ColumnType::Sideboard,
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

struct DeckTableModel::Pimpl : public virtual QAbstractTableModel
{
	Deck deck_;

	Pimpl()
	{
	}

	void reload()
	{
		beginResetModel();
		deck_.reload();
		endResetModel();
	}

	void load(const QString& filename)
	{
		beginResetModel();
		deck_.load(filename);
		endResetModel();
	}

	void save(const QString& filename)
	{
		deck_.save(filename);
	}

	int getQuantity(const int dataRowIndex) const
	{
		return deck_.getQuantity(dataRowIndex);
	}

	void setQuantity(const int dataRowIndex, const int newQuantity)
	{
		beginResetModel();
		deck_.setQuantity(dataRowIndex, newQuantity);
		endResetModel();
	}

	int getSideboard(const int dataRowIndex) const
	{
		return deck_.getSideboard(dataRowIndex);
	}

	void setSideboard(const int dataRowIndex, const int newSideboard)
	{
		beginResetModel();
		deck_.setSideboard(dataRowIndex, newSideboard);
		endResetModel();
	}

	virtual int rowCount(const QModelIndex& = QModelIndex()) const
	{
		return deck_.getNumRows();
	}

	virtual int columnCount(const QModelIndex& = QModelIndex()) const
	{
		return static_cast<int>(DECKTABLE_COLUMNS.size());
	}

	virtual QVariant data(const QModelIndex& index, int role) const
	{
		if (index.isValid())
		{
			if (role == Qt::DisplayRole)
			{
				if (index.row() < rowCount() && index.column() < columnCount())
				{
					const QVariant& ret = deck_.get(index.row(), DECKTABLE_COLUMNS[index.column()]);
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
					return static_cast<QString>(DECKTABLE_COLUMNS[section]);
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

DeckTableModel::DeckTableModel()
	: pimpl_(new Pimpl())
{
	setSourceModel(pimpl_.data());
}

DeckTableModel::~DeckTableModel()
{
}

void DeckTableModel::reload()
{
	pimpl_->reload();
}

void DeckTableModel::load(const QString& filename)
{
	pimpl_->load(filename);
}

void DeckTableModel::save(const QString& filename)
{
	pimpl_->save(filename);
}

const QString& DeckTableModel::getFilename() const
{
	return pimpl_->deck_.getFilename();
}

int DeckTableModel::getQuantity(const int dataRowIndex) const
{
	return pimpl_->getQuantity(dataRowIndex);
}

void DeckTableModel::setQuantity(const int dataRowIndex, const int newQuantity)
{
	pimpl_->setQuantity(dataRowIndex, newQuantity);
}

int DeckTableModel::getSideboard(const int dataRowIndex) const
{
	return pimpl_->getSideboard(dataRowIndex);
}

void DeckTableModel::setSideboard(const int dataRowIndex, const int newSideboard)
{
	pimpl_->setSideboard(dataRowIndex, newSideboard);
}

int DeckTableModel::getDataRowIndex(const QModelIndex& proxyIndex) const
{
	QModelIndex sourceIndex = mapToSource(proxyIndex);
	return pimpl_->deck_.getDataRowIndex(sourceIndex.row());
}

int DeckTableModel::getRowIndex(const int dataRowIndex) const
{
	return pimpl_->deck_.getRowIndex(dataRowIndex);
}

int DeckTableModel::columnToIndex(const mtg::ColumnType& column) const
{
	auto it = find(DECKTABLE_COLUMNS.begin(), DECKTABLE_COLUMNS.end(), column);
	if (it != DECKTABLE_COLUMNS.end())
	{
		return (it - DECKTABLE_COLUMNS.begin());
	}
	return -1;
}
