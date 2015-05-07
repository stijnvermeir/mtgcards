#include "filtermodel.h"

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

QModelIndex FilterModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!parent.isValid())
	{
		return createIndex(row, column, rootNode_.get());
	}
	else
	{
		FilterNode* parentNode = reinterpret_cast<FilterNode*>(parent.internalPointer());
		if (parentNode && row >= 0 && static_cast<size_t>(row) < parentNode->getChildren().size())
		{
			FilterNode* childNode = parentNode->getChildren()[static_cast<size_t>(row)].get();
			return createIndex(row, column, childNode);
		}
	}
	return QModelIndex();
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
		return createIndex(0, 0, parentNode);
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
	return Column::COUNT;
}

QVariant FilterModel::data(const QModelIndex& index, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	const FilterNode* node = nullptr;
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
		if (index.column() == Column::Type)
		{
			if (node->getType() == FilterNode::Type::LEAF && node->getFilter().function)
			{
				 return node->getFilter().function->getId();
			}
			else
			{
				return static_cast<QString>(node->getType());
			}
		}
		else
		if (index.column() == Column::Field)
		{
			if (node->getType() == FilterNode::Type::LEAF)
			{
				return static_cast<QString>(node->getFilter().column);
			}
		}
		else
		if (index.column() == Column::Filter)
		{
			if (node->getType() == FilterNode::Type::LEAF && node->getFilter().function)
			{
				return node->getFilter().function->getDescription();
			}
		}
	}
	return QVariant();
}

QVariant FilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	if (orientation != Qt::Horizontal)
	{
		return QVariant();
	}

	if (section == Column::Type)
	{
		return "Type";
	}

	if (section == Column::Field)
	{
		return "Field";
	}

	if (section == Column::Filter)
	{
		return "Filter";
	}

	return QVariant();
}

Qt::ItemFlags FilterModel::flags(const QModelIndex& index) const
{
	FilterNode* node = reinterpret_cast<FilterNode*>(index.internalPointer());
	if (node)
	{
		return (Qt::ItemIsEditable | QAbstractItemModel::flags(index));
	}
	return QAbstractItemModel::flags(index);
}

void FilterModel::setFilterRootNode(const FilterNode::Ptr& rootNode)
{
	beginResetModel();
	rootNode_ = rootNode;
	endResetModel();
}

const FilterNode::Ptr& FilterModel::getFilterRootNode() const
{
	return rootNode_;
}

void FilterModel::addNode(FilterNode::Ptr& node, const QModelIndex& index)
{
	beginResetModel();
	FilterNode* n = reinterpret_cast<FilterNode*>(index.internalPointer());
	if (n)
	{
		FilterNode* parentNode = n;
		if (n->getType() == FilterNode::Type::LEAF)
		{
			parentNode = n->getParent().get();
			if (parentNode == nullptr)
			{
				auto oldRoot = rootNode_;
				rootNode_ = FilterNode::create();
				rootNode_->setType(FilterNode::Type::AND);
				rootNode_->addChild(oldRoot);
				parentNode = rootNode_.get();
			}
		}
		parentNode->addChild(node);
	}
	else
	{
		if (rootNode_ == nullptr)
		{
			rootNode_ = node;
		}
	}
	endResetModel();
}

void FilterModel::deleteNode(const QModelIndex& index)
{
	FilterNode* node = reinterpret_cast<FilterNode*>(index.internalPointer());
	if (node)
	{
		beginResetModel();
		FilterNode* parentNode = node->getParent().get();
		if (parentNode == nullptr)
		{
			rootNode_ = FilterNode::Ptr();
		}
		else
		{
			auto n = node->shared_from_this();
			parentNode->removeChild(n);
		}
		endResetModel();
	}
}
