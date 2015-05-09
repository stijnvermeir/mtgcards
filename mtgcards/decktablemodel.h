#pragma once

#include "magicsortfilterproxymodel.h"

#include <QScopedPointer>
#include <QString>

class DeckTableModel : public MagicSortFilterProxyModel
{
public:

	DeckTableModel(const QString& filename);
	~DeckTableModel();

	void reload();
	void save(const QString& filename);
	const QString& getFilename() const;
	QString getDisplayName() const;
	bool isDeckActive() const;
	void setDeckActive(const bool active);
	bool hasUnsavedChanges() const;

	int getQuantity(const int dataRowIndex) const;
	void setQuantity(const int dataRowIndex, const int newQuantity);

	int getDataRowIndex(const QModelIndex& proxyIndex) const;
	int getRowIndex(const int dataRowIndex) const;

	virtual int columnToIndex(const mtg::ColumnType& column) const;
	mtg::ColumnType columnIndexToType(const int columnIndex) const;

private:
	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
