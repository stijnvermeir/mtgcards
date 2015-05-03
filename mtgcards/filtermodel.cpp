#include "filtermodel.h"

#include <QList>
#include <QDebug>

using namespace std;

FilterModel::FilterModel(QObject* parent)
	: QAbstractItemModel(parent)
	, rootNode_(nullptr)
{
}

FilterModel::~FilterModel()
{
}

void FilterModel::setFilterRootNode(const FilterNode::Ptr& rootNode)
{
	rootNode_ = rootNode;
}

const FilterNode::Ptr& FilterModel::getFilterRootNode() const
{
	return rootNode_;
}

QModelIndex FilterModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!parent.isValid())
	{
		return createIndex(row, column, rootNode_.get());
	}
	else
	{
		FilterNode* parentNode = reinterpret_cast<FilterNode*>(parent.internalPointer());
		FilterNode* childNode = parentNode->getChildren()[static_cast<size_t>(row)].get();
		return createIndex(row, column, reinterpret_cast<void*>(childNode));
	}
}

QModelIndex FilterModel::parent(const QModelIndex& child) const
{
	if (!child.isValid())
	{
		return QModelIndex();
	}

	FilterNode* childNode = reinterpret_cast<FilterNode*>(child.internalPointer());
	if (childNode == nullptr)
	{
		return QModelIndex();
	}
	FilterNode* parentNode = childNode->getParent().get();
	if (parentNode == nullptr)
	{
		return QModelIndex();
	}
	if (parentNode == rootNode_.get())
	{
		return createIndex(0, 0, reinterpret_cast<void*>(parentNode));
	}
	FilterNode* parentParentNode = parentNode->getParent().get();
	for (int row = 0; row < static_cast<int>(parentParentNode->getChildren().size()); ++row)
	{
		if (parentParentNode->getChildren().at(row).get() == parentNode)
		{
			return createIndex(row, 0, reinterpret_cast<void*>(parentNode));
		}
	}
	return QModelIndex();
}

int FilterModel::rowCount(const QModelIndex& parent) const
{
	if (!parent.isValid())
	{
		return 1;
	}
	FilterNode* node = reinterpret_cast<FilterNode*>(parent.internalPointer());
	if (node)
	{
		return static_cast<int>(node->getChildren().size());
	}
	return 0;
}

int FilterModel::columnCount(const QModelIndex& /*parent*/) const
{
	return 1;
}

QVariant FilterModel::data(const QModelIndex& index, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	const FilterNode* node;
	if (index.isValid())
	{
		node = reinterpret_cast<const FilterNode*>(index.internalPointer());
	}
	else
	{
		node = rootNode_.get();
	}
	if (node)
	{
		if (node->getType() == FilterNode::Type::AND)
		{
			return "AND";
		}
		else
		if (node->getType() == FilterNode::Type::OR)
		{
			return "OR";
		}
		else
		if (node->getType() == FilterNode::Type::LEAF)
		{
			QString val = node->getFilter().column;
			val += ": ";
			val += node->getFilter().function->getDescription();
			return val;
		}
	}
	return QVariant();
}

Qt::ItemFlags FilterModel::flags(const QModelIndex& index) const
{
	FilterNode* node = reinterpret_cast<FilterNode*>(index.internalPointer());
	if (node)
	{
		if (node->getType() == FilterNode::Type::LEAF && node->getFilter().function)
		{
			return (Qt::ItemIsEditable | QAbstractItemModel::flags(index));
		}
	}
	return QAbstractItemModel::flags(index);
}
