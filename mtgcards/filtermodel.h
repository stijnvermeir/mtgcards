#pragma once

#include "filter.h"

#include <QAbstractItemModel>

class FilterModel : public QAbstractItemModel
{
public:
	FilterModel(QObject* parent = 0);
	~FilterModel();

	void setFilterRootNode(const FilterNode::Ptr& rootNode);
	const FilterNode::Ptr& getFilterRootNode() const;

	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex& child) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const;

	void addNode(FilterNode::Ptr& node, const QModelIndex& index);
	void deleteNode(const QModelIndex& index);

private:
	FilterNode::Ptr rootNode_;
};
