#pragma once

#include <QAbstractItemModel>

class FilterModel : public QAbstractItemModel
{
public:
	FilterModel(QObject* parent = 0);
	~FilterModel();

	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex& child) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

private:
	struct Node;
	Node* rootNode_;
};
