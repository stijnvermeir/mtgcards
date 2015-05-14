#include "decktablemodel.h"
#include "deckmanager.h"

#include <QAbstractTableModel>
#include <QDebug>

using namespace std;

namespace {

const QVector<mtg::ColumnType> DECKTABLE_COLUMNS =
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
	QSharedPointer<Deck> deck_;

	Pimpl(const QString& filename)
		: deck_(DeckManager::instance().getDeck(filename))
	{
	}

	void reload()
	{
		beginResetModel();
		deck_->reload();
		endResetModel();
	}

	void save(const QString& filename)
	{
		QString previousFilename = deck_->getFilename();
		deck_->save(filename);
		// in case of Save As, load the copied from deck back into memory
		if (previousFilename != filename && !previousFilename.isEmpty())
		{
			DeckManager::instance().getDeck(previousFilename);
		}
	}

	int getQuantity(const int dataRowIndex) const
	{
		return deck_->getQuantity(dataRowIndex);
	}

	void setQuantity(const int dataRowIndex, const int newQuantity)
	{
		beginResetModel();
		deck_->setQuantity(dataRowIndex, newQuantity);
		endResetModel();
	}

	virtual int rowCount(const QModelIndex& = QModelIndex()) const
	{
		return deck_->getNumRows();
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
					const QVariant& ret = deck_->get(index.row(), DECKTABLE_COLUMNS[index.column()]);
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
					int dataRowIndex = deck_->getDataRowIndex(index.row());
					if (DECKTABLE_COLUMNS[index.column()] == mtg::ColumnType::Sideboard)
					{
						if (deck_->getSideboard(dataRowIndex) != value.toInt())
						{
							deck_->setSideboard(dataRowIndex, value.toInt());
							emit dataChanged(index, index);
							return true;
						}
					}
					if (DECKTABLE_COLUMNS[index.column()] == mtg::ColumnType::Quantity)
					{
						if (deck_->getQuantity(dataRowIndex) != value.toInt())
						{
							deck_->setQuantity(dataRowIndex, value.toInt());
							emit dataChanged(index, index);
							return true;
						}
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
					return DECKTABLE_COLUMNS[section].getDisplayName();
				}
			}
		}
		return QVariant();
	}

	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		if (index.column() < columnCount())
		{
			if (DECKTABLE_COLUMNS[index.column()] == mtg::ColumnType::Quantity ||
				DECKTABLE_COLUMNS[index.column()] == mtg::ColumnType::Sideboard)
			{
				return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
			}
		}
		return QAbstractTableModel::flags(index);
	}
};

DeckTableModel::DeckTableModel(const QString& filename)
	: pimpl_(new Pimpl(filename))
{
	setSourceModel(pimpl_.data());
}

DeckTableModel::~DeckTableModel()
{
	DeckManager::instance().closeDeck(pimpl_->deck_);
}

void DeckTableModel::reload()
{
	pimpl_->reload();
}

void DeckTableModel::save(const QString& filename)
{
	pimpl_->save(filename);
}

void DeckTableModel::setDeckActive(const bool active)
{
	pimpl_->deck_->setActive(active);
}

void DeckTableModel::setQuantity(const int dataRowIndex, const int newQuantity)
{
	pimpl_->setQuantity(dataRowIndex, newQuantity);
}

const Deck& DeckTableModel::deck() const
{
	return *pimpl_->deck_.data();
}

int DeckTableModel::getDataRowIndex(const QModelIndex& proxyIndex) const
{
	QModelIndex sourceIndex = mapToSource(proxyIndex);
	return pimpl_->deck_->getDataRowIndex(sourceIndex.row());
}

int DeckTableModel::getRowIndex(const int dataRowIndex) const
{
	return pimpl_->deck_->getRowIndex(dataRowIndex);
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

mtg::ColumnType DeckTableModel::columnIndexToType(const int columnIndex) const
{
	if (columnIndex >= 0 && columnIndex < sourceModel()->columnCount())
	{
		return DECKTABLE_COLUMNS[columnIndex];
	}
	return mtg::ColumnType::UNKNOWN;
}
