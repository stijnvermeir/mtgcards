#pragma once

#include "magiccolumntype.h"

#include <QVariant>
#include <QScopedPointer>
#include <QtSql/QSqlDatabase>

namespace mtg {

class Collection
{
public:
	static Collection& instance();

	QSqlDatabase getConnection();

	void load();

	int getNumRows() const;
	int getNumCards() const;
	QVariant get(const int row, const ColumnType& column) const;
	int getDataRowIndex(const int row) const;
	int getRowIndex(const int dataRowIndex) const;

	int getQuantity(const int dataRowIndex) const;
    int getQuantityAll(const int dataRowIndex) const;
	void setQuantity(const int dataRowIndex, const int newQuantity);

	void setUsedCount(const int dataRowIndex, const int usedCount);

	void set(const int row, const ColumnType& column, const QVariant& data);

private:
	Collection();
	~Collection();

	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};

} // namespace mtg
