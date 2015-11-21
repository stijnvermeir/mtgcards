#include "magicsortfilterproxymodel.h"

#include "manacost.h"
#include "magiccarddata.h"
#include "onlinedatacache.h"
#include "settings.h"

#include <mkm/exception.h>

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

void MagicSortFilterProxyModel::fetchOnlineData(const QModelIndexList& selectedRows)
{
	try
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
	catch (const mkm::MkmException& e)
	{
		QMessageBox msg(QMessageBox::Critical, "Error", e.getErrorMessage());
		msg.setDetailedText(e.getErrorDetails());
		msg.exec();
	}
}

QVariant MagicSortFilterProxyModel::data(const QModelIndex& index, int role) const
{
	QVariant d = QSortFilterProxyModel::data(index, role);
	if (d.canConvert<ManaCost>() && role == Qt::ToolTipRole)
	{
		return qvariant_cast<ManaCost>(d).getText();
	}
	/*
	if (index.column() == columnToIndex(mtg::ColumnType::Name) && role == Qt::ToolTipRole)
	{
		auto picInfo = mtg::CardData::instance().getPictureInfo(getDataRowIndex(index));
		if (picInfo.missing.empty())
		{
			return QString("<img src=\"") + picInfo.filenames.front() + "\" />";
		}
	}
	*/
	return d;
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
