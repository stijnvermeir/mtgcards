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
	beginResetModel();
	rootNode_ = rootNode;
	endResetModel();
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
	return 3;
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
		if (index.column() == 0)
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
				 return static_cast<QString>(node->getFilter().function->getType());
			}
		}
		else
		if (index.column() == 1)
		{
			if (node->getType() == FilterNode::Type::LEAF)
			{
				return static_cast<QString>(node->getFilter().column);
			}
		}
		else
		if (index.column() == 2)
		{
			if (node->getType() == FilterNode::Type::LEAF)
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

	if (section == 0)
	{
		return "Type";
	}

	if (section == 1)
	{
		return "Field";
	}

	if (section == 2)
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
