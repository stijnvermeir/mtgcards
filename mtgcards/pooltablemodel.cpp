#include "pooltablemodel.h"

#include "magiccarddata.h"
#include "magiccollection.h"
#include "deckmanager.h"
#include "util.h"

#include <QAbstractTableModel>
#include <QDebug>

namespace {

const QVector<mtg::ColumnType> POOLTABLE_COLUMNS =
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
	mtg::ColumnType::IsLatestPrint,
	mtg::ColumnType::PriceLowest,
	mtg::ColumnType::PriceLowestFoil,
	mtg::ColumnType::PriceAverage,
	mtg::ColumnType::PriceTrend,
	mtg::ColumnType::Id,
    mtg::ColumnType::Tags,
    mtg::ColumnType::ColorIdentity,
    mtg::ColumnType::LegalityStandard,
    mtg::ColumnType::LegalityModern,
    mtg::ColumnType::LegalityLegacy,
    mtg::ColumnType::LegalityVintage,
    mtg::ColumnType::LegalityCommander,
    mtg::ColumnType::Owned,
    mtg::ColumnType::OwnedAll,
    mtg::ColumnType::Used,
    mtg::ColumnType::UsedAll,
	mtg::ColumnType::NotOwned
};

} // namespace

struct PoolTableModel::Pimpl : public virtual QAbstractTableModel
{
	Pimpl()
	{
	}

	virtual int rowCount(const QModelIndex& ) const
	{
		return mtg::CardData::instance().getNumRows();
	}

	virtual int columnCount(const QModelIndex& = QModelIndex()) const
	{
		return POOLTABLE_COLUMNS.size();
	}

	virtual QVariant data(const QModelIndex& index, int role) const
	{
		if (index.isValid())
		{
			if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
			{
				if (index.row() < mtg::CardData::instance().getNumRows() && index.column() < columnCount())
				{
                    int dataRowIndex = index.row();
                    auto columnType = POOLTABLE_COLUMNS[index.column()];
                    if (columnType == mtg::ColumnType::Owned)
                    {
                        return mtg::Collection::instance().getQuantity(dataRowIndex);
                    }
                    if (columnType == mtg::ColumnType::OwnedAll)
                    {
                        if (role == Qt::ToolTipRole)
                        {
                            return Util::getOwnedAllTooltip(dataRowIndex);
                        }
                        return mtg::Collection::instance().getQuantityAll(dataRowIndex);
                    }
                    if (columnType == mtg::ColumnType::Used)
                    {
                        if (role == Qt::ToolTipRole)
                        {
                            return Util::getUsedTooltip(dataRowIndex);
                        }
                        return DeckManager::instance().getUsedCount(dataRowIndex);
                    }
                    if (columnType == mtg::ColumnType::UsedAll)
                    {
                        if (role == Qt::ToolTipRole)
                        {
                            return Util::getUsedAllTooltip(dataRowIndex);
                        }
                        return DeckManager::instance().getUsedAllCount(dataRowIndex);
                    }
                    if (columnType == mtg::ColumnType::NotOwned)
                    {
                        auto notOwned = DeckManager::instance().getUsedCount(dataRowIndex) - mtg::Collection::instance().getQuantity(dataRowIndex);
                        return ((notOwned > 0) ? notOwned : 0);
                    }
                    if (columnType == mtg::ColumnType::SetCode && role == Qt::ToolTipRole)
                    {
                        return mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::Set);
                    }
                    return mtg::CardData::instance().get(dataRowIndex, columnType);
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
					return POOLTABLE_COLUMNS[section].getDisplayName();
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
	setSourceModel(pimpl_.data());
}

PoolTableModel::~PoolTableModel()
{
}

int PoolTableModel::columnToIndex(const mtg::ColumnType& column) const
{
	auto it = std::find_if(POOLTABLE_COLUMNS.begin(), POOLTABLE_COLUMNS.end(), [&column](const mtg::ColumnType& c)
	{
		return c.value() == column.value();
	});
	if (it != POOLTABLE_COLUMNS.end())
	{
		return it - POOLTABLE_COLUMNS.begin();
	}
	return -1;
}

int PoolTableModel::getDataRowIndex(const QModelIndex& proxyIndex) const
{
	QModelIndex sourceIndex = mapToSource(proxyIndex);
	return sourceIndex.row();
}

mtg::ColumnType PoolTableModel::columnIndexToType(const int columnIndex) const
{
	if (columnIndex >= 0 && columnIndex < POOLTABLE_COLUMNS.size())
	{
		return POOLTABLE_COLUMNS[columnIndex];
	}
	return mtg::ColumnType::UNKNOWN;
}
