#include "magicsortfilterproxymodel.h"

#include "manacost.h"
#include "magiccarddata.h"
#include "settings.h"
#include "prices.h"

#include <QDebug>
#include <QUrl>
#include <QProgressDialog>
#include <QMessageBox>

#include <functional>

using namespace std;

const FilterNode::Ptr& MagicSortFilterProxyModel::getFilterRootNode() const
{
	return filterRootNode_;
}

void MagicSortFilterProxyModel::setFilterRootNode(const FilterNode::Ptr& rootNode)
{
	filterRootNode_ = rootNode;
	invalidateFilter();
}

void MagicSortFilterProxyModel::downloadCardArt(const QModelIndexList& selectedRows)
{
	QProgressDialog progress("Downloading card art ...", "Cancel", 0, selectedRows.size());
	progress.setWindowModality(Qt::WindowModal);
	progress.show();
	int i = 0;
	for (const QModelIndex& row : selectedRows)
	{
		int dataRowIndex = getDataRowIndex(row);
		mtg::CardData::instance().getPictureInfo(dataRowIndex, Settings::instance().getArtIsHighQuality(), true);
		progress.setValue(i++);
		if (progress.wasCanceled())
		{
			break;
		}
	}
	progress.setValue(selectedRows.size());
	QMessageBox::information(nullptr, "Download card art.", "Download finished.");
}

void MagicSortFilterProxyModel::fetchOnlineData(const QModelIndexList& selectedRows)
{
	QProgressDialog progress("Fetching online data ...", "Cancel", 0, selectedRows.size());
	progress.setWindowModality(Qt::WindowModal);
	int i = 0;
	for (const QModelIndex& row : selectedRows)
	{
		int dataRowIndex = getDataRowIndex(row);
		mtg::CardData::instance().fetchOnlineData(dataRowIndex);
		emit dataChanged(index(row.row(), 0), index(row.row(), columnCount() - 1));
		progress.setValue(i++);
		if (progress.wasCanceled())
		{
			break;
		}
	}
	progress.setValue(selectedRows.size());
}

QVariant MagicSortFilterProxyModel::data(const QModelIndex& index, int role) const
{
	QVariant d = QSortFilterProxyModel::data(index, role);
	if (d.canConvert<ManaCost>() && role == Qt::ToolTipRole)
	{
		return qvariant_cast<ManaCost>(d).getText();
	}
	if (index.column() == columnToIndex(mtg::ColumnType::Tags))
	{
		if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
		{
			return mtg::CardData::instance().get(getDataRowIndex(index), mtg::ColumnType::Tags).toStringList().join('/');
		}
		if (role == Qt::EditRole)
		{
			return mtg::CardData::instance().getCardTagCompletions(getDataRowIndex(index));
		}
	}
	if (d.type() == QVariant::StringList && (role == Qt::DisplayRole || role == Qt::ToolTipRole))
	{
		return d.toStringList().join("/");
	}
	return d;
}

bool MagicSortFilterProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.column() == columnToIndex(mtg::ColumnType::Tags) && role == Qt::EditRole)
	{
		mtg::CardData::instance().updateTags(getDataRowIndex(index), value.toString());
		emit dataChanged(index, index);
		return true;
	}
	if (index.column() == columnToIndex(mtg::ColumnType::Price) && role == Qt::EditRole)
	{
		auto uuid = mtg::CardData::instance().get(getDataRowIndex(index), mtg::ColumnType::Uuid);
		if (uuid.isValid() && value.isValid())
		{
			double price = value.toDouble();
			price = std::round(price * 100.0) / 100.0;
			Prices::instance().setPrice(uuid.toString(), price);
		}
		emit dataChanged(index, index);
		return true;
	}
	return QSortFilterProxyModel::setData(index, value, role);
}

Qt::ItemFlags MagicSortFilterProxyModel::flags(const QModelIndex& index) const
{
	if (index.column() == columnToIndex(mtg::ColumnType::Tags))
	{
		return Qt::ItemIsEditable | QSortFilterProxyModel::flags(index);
	}
	if (index.column() == columnToIndex(mtg::ColumnType::Price))
	{
		return Qt::ItemIsEditable | QSortFilterProxyModel::flags(index);
	}
	return QSortFilterProxyModel::flags(index);
}

bool MagicSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
	if (left.data().canConvert<ManaCost>())
	{
		return qvariant_cast<ManaCost>(left.data()) < qvariant_cast<ManaCost>(right.data());
	}
	return QSortFilterProxyModel::lessThan(left, right);
}

bool MagicSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& /*source_parent*/) const
{
	function<bool(const FilterNode::Ptr&)> applyFilter = [this, source_row, &applyFilter](const FilterNode::Ptr& node)
	{
		if (!node)
		{
			return true;
		}

		if (node->getType() == FilterNode::Type::AND)
		{
			for (const FilterNode::Ptr& child : node->getChildren())
			{
				if (!applyFilter(child))
				{
					return false;
				}
			}
			return true;
		}
		else
		if (node->getType() == FilterNode::Type::OR)
		{
			for (const FilterNode::Ptr& child : node->getChildren())
			{
				if (applyFilter(child))
				{
					return true;
				}
			}
			return false;
		}
		else
		if (node->getType() == FilterNode::Type::LEAF)
		{
			if (!node->getFilter().function)
			{
				return true;
			}
			int columnIndex = -1;
			if (node->getFilter().function->getColumnOverride() != mtg::ColumnType::UNKNOWN)
			{
				columnIndex = columnToIndex(node->getFilter().function->getColumnOverride());
			}
			else
			{
				columnIndex = columnToIndex(node->getFilter().column);
			}
			if (columnIndex < 0)
			{
				return true;
			}
			QVariant d = sourceModel()->data(sourceModel()->index(source_row, columnIndex));
			if (node->getFilter().negate)
			{
				return !(node->getFilter().function->apply(d));
			}
			return node->getFilter().function->apply(d);
		}
		return false;
	};

	FilterNode::Ptr local = filterRootNode_;
	FilterNode::Ptr global = Settings::instance().getGlobalFilter();
	FilterNode::Ptr node = FilterNode::create();
	node->setType(FilterNode::Type::AND);
	node->addChild(local);
	node->addChild(global);
	bool rv = applyFilter(node);
	node->removeChild(global);
	node->removeChild(local);
	return rv;
}
