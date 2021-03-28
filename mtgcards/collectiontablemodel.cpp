#include "collectiontablemodel.h"

#include "magiccollection.h"
#include "deckmanager.h"
#include "util.h"

#include <QAbstractTableModel>
#include <QDebug>

namespace {

const QVector<mtg::ColumnType> COLLECTIONTABLE_COLUMNS =
{
    mtg::ColumnType::SetName,
	mtg::ColumnType::SetCode,
	mtg::ColumnType::SetReleaseDate,
    mtg::ColumnType::SetType,
	mtg::ColumnType::Name,
	mtg::ColumnType::Names,
    mtg::ColumnType::ManaCost,
	mtg::ColumnType::CMC,
	mtg::ColumnType::Color,
    mtg::ColumnType::ColorIdentity,
    mtg::ColumnType::LegalityCommander,
    mtg::ColumnType::CanBeCommander,
    mtg::ColumnType::IsCompanion,
	mtg::ColumnType::Type,
	mtg::ColumnType::Rarity,
	mtg::ColumnType::Text,
	mtg::ColumnType::Power,
	mtg::ColumnType::Toughness,
	mtg::ColumnType::Loyalty,
    mtg::ColumnType::Quantity,
    mtg::ColumnType::QuantityAll,
    mtg::ColumnType::Used,
    mtg::ColumnType::UsedAll,
    mtg::ColumnType::NotOwned,
    mtg::ColumnType::Price,
    mtg::ColumnType::Tags,
    mtg::ColumnType::IsAlternative,
    mtg::ColumnType::IsFullArt,
    mtg::ColumnType::IsExtendedArt,
    mtg::ColumnType::IsPromo,
    mtg::ColumnType::IsReprint,
    mtg::ColumnType::IsLatestPrint,
    mtg::ColumnType::Layout,
    mtg::ColumnType::Border
};

const QVector<mtg::ColumnType>& GetColumns()
{
	static QVector<mtg::ColumnType> columns;
	static bool ready = false;
	if (!ready)
	{
		columns.reserve(COLLECTIONTABLE_COLUMNS.size());
		for (const mtg::ColumnType& column : COLLECTIONTABLE_COLUMNS)
		{
			columns.push_back(column);
		}
		ready = true;
	}
	return columns;
}

} // namespace

struct CollectionTableModel::Pimpl : public virtual QAbstractTableModel
{
	Pimpl()
	{
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
                    if (role == Qt::ToolTipRole && GetColumns()[index.column()] == mtg::ColumnType::QuantityAll)
                    {
                        return Util::getOwnedAllTooltip(mtg::Collection::instance().getDataRowIndex(index.row()));
                    }
					if (role == Qt::ToolTipRole && GetColumns()[index.column()] == mtg::ColumnType::Used)
					{
                        return Util::getUsedTooltip(mtg::Collection::instance().getDataRowIndex(index.row()));
					}
                    if (role == Qt::ToolTipRole && GetColumns()[index.column()] == mtg::ColumnType::UsedAll)
                    {
                        return Util::getUsedAllTooltip(mtg::Collection::instance().getDataRowIndex(index.row()));
                    }
                    if (GetColumns()[index.column()] == mtg::ColumnType::SetCode && role == Qt::ToolTipRole)
                    {
						return mtg::Collection::instance().get(index.row(), mtg::ColumnType::SetName);
                    }
					return mtg::Collection::instance().get(index.row(), GetColumns()[index.column()]);
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
					if (GetColumns()[index.column()] == mtg::ColumnType::Quantity)
					{
						if (mtg::Collection::instance().getQuantity(dataRowIndex) != value.toInt() && value.toInt() >= 0)
						{
							mtg::Collection::instance().setQuantity(dataRowIndex, value.toInt());
							emit dataChanged(index, index);
							return true;
						}
					}
					else
					if (GetColumns()[index.column()] == mtg::ColumnType::Used)
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
					return GetColumns()[section].getDisplayName();
				}
			}
			if (role == Qt::UserRole)
			{
				if (section >= 0 && section < columnCount())
				{
					return QString(GetColumns()[section]);
				}
			}
		}
		return QVariant();
	}

	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		if (index.column() < columnCount())
		{
			if (GetColumns()[index.column()] == mtg::ColumnType::Quantity)
			{
				return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
			}
		}
		return QAbstractTableModel::flags(index);
	}

	void updateUsedCount()
	{
		for (int row = 0; row < rowCount(); ++row)
		{
			int dataRowIndex = mtg::Collection::instance().getDataRowIndex(row);
			int usedCount = DeckManager::instance().getUsedCount(dataRowIndex);
			mtg::Collection::instance().setUsedCount(dataRowIndex, usedCount);
		}
		int column = columnToIndex(mtg::ColumnType::Used);
		emit dataChanged(index(0, column), index(rowCount() - 1, column));
	}

	int columnToIndex(const mtg::ColumnType& column) const
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
	return pimpl_->columnToIndex(column);
}

int CollectionTableModel::getDataRowIndex(const QModelIndex& proxyIndex) const
{
	QModelIndex sourceIndex = mapToSource(proxyIndex);
	return mtg::Collection::instance().getDataRowIndex(sourceIndex.row());
}

mtg::ColumnType CollectionTableModel::columnIndexToType(const int columnIndex) const
{
	if (columnIndex >= 0 && columnIndex < sourceModel()->columnCount())
	{
		return GetColumns()[columnIndex];
	}
	return mtg::ColumnType::UNKNOWN;
}

void CollectionTableModel::updateUsedCount()
{
	pimpl_->updateUsedCount();
}
