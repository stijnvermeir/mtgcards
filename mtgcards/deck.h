#pragma once

#include "magiccolumntype.h"

#include <QVariant>
#include <QString>
#include <QScopedPointer>

class Deck
{
public:
	Deck();
	Deck(const QString& file);
	~Deck();

	void reload();
	void load(const QString& filename);
	void save(const QString& filename);
	const QString& getFilename() const;

	int getNumRows() const;
	int getNumCards() const;
	const QVariant& get(const int row, const mtg::ColumnType& column) const;
	int getDataRowIndex(const int row) const;
	int getRowIndex(const int dataRowIndex) const;

	int getQuantity(const int dataRowIndex) const;
	void setQuantity(const int dataRowIndex, const int newQuantity);

	int getSideboard(const int dataRowIndex) const;
	void setSideboard(const int dataRowIndex, const int newSideboard);

private:
	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
