#include "decktablemodel.h"

#include "deckmanager.h"
#include "magiccollection.h"
#include "util.h"

#include <QAbstractTableModel>
#include <QDebug>

namespace {

const QVector<mtg::ColumnType> DECKTABLE_COLUMNS =
{
	mtg::ColumnType::Set,
	mtg::ColumnType::SetCode,
	mtg::ColumnType::SetGathererCode,
	mtg::ColumnType::SetOldCode,
	mtg::ColumnType::SetReleaseDate,
	mtg::ColumnType::SetType,
	mtg::ColumnType::Block,
	mtg::ColumnType::OnlineOnly,
	mtg::ColumnType::Border,
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
	mtg::ColumnType::ImageName,
	mtg::ColumnType::Owned,
	mtg::ColumnType::PriceLowest,
	mtg::ColumnType::PriceLowestFoil,
	mtg::ColumnType::PriceAverage,
	mtg::ColumnType::PriceTrend,
	mtg::ColumnType::Id,
	mtg::ColumnType::Tags,
    mtg::ColumnType::NotOwned,
    mtg::ColumnType::ColorIdentity,
    mtg::ColumnType::LegalityStandard,
    mtg::ColumnType::LegalityModern,
    mtg::ColumnType::LegalityLegacy,
    mtg::ColumnType::LegalityVintage,
    mtg::ColumnType::LegalityCommander,
    mtg::ColumnType::OwnedAll,
    mtg::ColumnType::IsCompanion,
    mtg::ColumnType::CanBeCommander,
    mtg::ColumnType::IsAlternative,
    mtg::ColumnType::IsFullArt,
    mtg::ColumnType::IsPromo,
    mtg::ColumnType::IsReprint
};

const QVector<mtg::ColumnType>& GetColumns()
{
	static QVector<mtg::ColumnType> columns;
	static bool ready = false;
	if (!ready)
	{
		columns.reserve(DECKTABLE_COLUMNS.size());
		for (const mtg::ColumnType& column : DECKTABLE_COLUMNS)
		{
			columns.push_back(column);
		}
		ready = true;
	}
	return columns;
}

} // namespace

struct DeckTableModel::Pimpl : public virtual QAbstractTableModel
{
	QSharedPointer<Deck> deck_;

	Pimpl(const QString& filename)
		: deck_(DeckManager::instance().getDeck(filename))
	{
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

	void setSideboard(const int dataRowIndex, const int newSideboard)
	{
		beginResetModel();
		deck_->setSideboard(dataRowIndex, newSideboard);
		endResetModel();
	}

	virtual int rowCount(const QModelIndex& = QModelIndex()) const
	{
		return deck_->getNumRows();
	}

	virtual int columnCount(const QModelIndex& = QModelIndex()) const
	{
		return GetColumns().size();
	}

	virtual QVariant data(const QModelIndex& index, int role) const
	{
		if (index.isValid())
		{
			if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::ToolTipRole)
			{
				if (index.row() < rowCount() && index.column() < columnCount())
				{
					if (GetColumns()[index.column()] == mtg::ColumnType::Owned)
					{
						int dataRowIndex = deck_->getDataRowIndex(index.row());
						if (role == Qt::ToolTipRole)
						{
                            return Util::getUsedTooltip(dataRowIndex);
						}
						return mtg::Collection::instance().getQuantity(dataRowIndex);
					}
                    if (GetColumns()[index.column()] == mtg::ColumnType::OwnedAll)
                    {
                        int dataRowIndex = deck_->getDataRowIndex(index.row());
                        if (role == Qt::ToolTipRole)
                        {
                            return Util::getUsedAllTooltip(dataRowIndex);
                        }
                        return mtg::Collection::instance().getQuantityAll(dataRowIndex);
                    }
					if (GetColumns()[index.column()] == mtg::ColumnType::NotOwned)
					{
						auto dataRowIndex = deck_->getDataRowIndex(index.row());
						auto rowIndex = mtg::Collection::instance().getRowIndex(dataRowIndex);
						if (rowIndex == -1)
						{
							return deck_->getQuantity(dataRowIndex);
						}
						return mtg::Collection::instance().get(rowIndex, mtg::ColumnType::NotOwned);
					}
                    if (GetColumns()[index.column()] == mtg::ColumnType::SetCode && role == Qt::ToolTipRole)
                    {
                        return deck_->get(index.row(), mtg::ColumnType::Set);
                    }
					return deck_->get(index.row(), GetColumns()[index.column()]);
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
					if (GetColumns()[index.column()] == mtg::ColumnType::Sideboard)
					{
						if (deck_->getSideboard(dataRowIndex) != value.toInt())
						{
							deck_->setSideboard(dataRowIndex, value.toInt());
							emit dataChanged(index, index);
							return true;
						}
					}
					else
					if (GetColumns()[index.column()] == mtg::ColumnType::Quantity)
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
					return GetColumns()[section].getDisplayName();
				}
			}
		}
		return QVariant();
	}

	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		if (index.column() < columnCount())
		{
			if (GetColumns()[index.column()] == mtg::ColumnType::Quantity ||
			    GetColumns()[index.column()] == mtg::ColumnType::Sideboard)
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

void DeckTableModel::setSideboard(const int dataRowIndex, const int newSideboard)
{
	pimpl_->setSideboard(dataRowIndex, newSideboard);
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
	const auto& columns = GetColumns();
	auto it = std::find_if(columns.begin(), columns.end(), [&column](const mtg::ColumnType& c)
	{
		return c.value() == column.value();
	});
	if (it != columns.end())
	{
		return it - columns.begin();
	}
	return -1;
}

mtg::ColumnType DeckTableModel::columnIndexToType(const int columnIndex) const
{
	if (columnIndex >= 0 && columnIndex < sourceModel()->columnCount())
	{
		return GetColumns()[columnIndex];
	}
	return mtg::ColumnType::UNKNOWN;
}
