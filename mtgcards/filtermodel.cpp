#include "filtermodel.h"

#include <QList>
#include <QDebug>

using namespace std;

struct FilterModel::Node
{
	QList<Node*> children;
	Node* parent;
	QString data;

	explicit Node(Node* parentNode = nullptr)
		: children()
		, parent(parentNode)
	{
	}

	~Node()
	{
		qDeleteAll(children);
	}

	int row()
	{
		if (parent)
		{
			return parent->children.indexOf(this);
		}
		return 0;
	}
};

FilterModel::FilterModel(QObject* parent)
	: QAbstractItemModel(parent)
	, rootNode_(nullptr)
{
	rootNode_ = new Node();

	Node* root = new Node(rootNode_);
	root->data = "OR";
	Node* child1 = new Node(root);
	child1->data = "AND";
	Node* child1a = new Node(child1);
	child1a->data = "Rarity = Mythic Rare";
	child1->children.push_back(child1a);
	Node* child1b = new Node(child1);
	child1b->data = "CMC = 4";
	child1->children.push_back(child1b);
	root->children.push_back(child1);
	Node* child2 = new Node(root);
	child2->data = "It's an elf";
	root->children.push_back(child2);

	rootNode_->children.push_back(root);
}

FilterModel::~FilterModel()
{
	if (rootNode_)
	{
		delete rootNode_;
	}
}

QModelIndex FilterModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent))
	{
		return QModelIndex();
	}

	Node* parentNode;
	if (!parent.isValid())
	{
		parentNode = rootNode_;
	}
	else
	{
		parentNode = static_cast<Node*>(parent.internalPointer());
	}

	Node* childNode = parentNode->children.value(row);
	if (childNode)
	{
		return createIndex(row, column, childNode);
	}
	else
	{
		return QModelIndex();
	}
}

QModelIndex FilterModel::parent(const QModelIndex& child) const
{
	if (!child.isValid())
	{
		return QModelIndex();
	}

	Node* parentNode = static_cast<Node*>(child.internalPointer())->parent;
	if (parentNode == rootNode_)
	{
		return QModelIndex();
	}
	return createIndex(parentNode->row(), 0, parentNode);
}

int FilterModel::rowCount(const QModelIndex& parent) const
{
	Node* parentNode;
	if (!parent.isValid())
	{
		parentNode = rootNode_;
	}
	else
	{
		parentNode = static_cast<Node*>(parent.internalPointer());
	}
	return parentNode->children.count();
}

int FilterModel::columnCount(const QModelIndex& /*parent*/) const
{
	return 1;
}

QVariant FilterModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	Node* node = static_cast<Node*>(index.internalPointer());
	return node->data;
}
