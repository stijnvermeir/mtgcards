#pragma once

#include "magicsortfilterproxymodel.h"

#include <QScopedPointer>
#include <QString>

class Deck;

class DeckTableModel : public MagicSortFilterProxyModel
{
public:

	DeckTableModel(const QString& filename);
	~DeckTableModel();

	void save(const QString& filename);
	void setDeckActive(const bool active);
	void setQuantity(const int dataRowIndex, const int newQuantity);
	void setSideboard(const int dataRowIndex, const int newSideboard);
	void setCommander(const int dataRowIndex, const bool commander);
	const Deck& deck() const;

	virtual int getDataRowIndex(const QModelIndex& proxyIndex) const;
	int getRowIndex(const int dataRowIndex) const;

	virtual int columnToIndex(const mtg::ColumnType& column) const;
	mtg::ColumnType columnIndexToType(const int columnIndex) const;

private:
	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
