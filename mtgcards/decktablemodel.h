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

	void reload();
	void save(const QString& filename);
	void setDeckActive(const bool active);
	void setQuantity(const int dataRowIndex, const int newQuantity);
	const Deck& deck() const;

	int getDataRowIndex(const QModelIndex& proxyIndex) const;
	int getRowIndex(const int dataRowIndex) const;

	virtual int columnToIndex(const mtg::ColumnType& column) const;
	mtg::ColumnType columnIndexToType(const int columnIndex) const;

private:
	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
