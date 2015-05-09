#pragma once

#include "magicsortfilterproxymodel.h"

#include <QScopedPointer>
#include <QString>

class DeckTableModel : public MagicSortFilterProxyModel
{
public:

	DeckTableModel();
	~DeckTableModel();

	void reload();
	void load(const QString& filename);
	void save(const QString& filename);
	const QString& getFilename() const;

	int getQuantity(const int dataRowIndex) const;
	void setQuantity(const int dataRowIndex, const int newQuantity);

	int getSideboard(const int dataRowIndex) const;
	void setSideboard(const int dataRowIndex, const int newSideboard);

	int getDataRowIndex(const QModelIndex& proxyIndex) const;
	int getRowIndex(const int dataRowIndex) const;

	virtual int columnToIndex(const mtg::ColumnType& column) const;

private:
	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
