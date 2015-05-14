#pragma once

#include "magiccolumntype.h"

#include <QVariant>
#include <QScopedPointer>

namespace mtg {

class Collection
{
public:
	static Collection& instance();

	void load();
	void save();

	int getNumRows() const;
	int getNumCards() const;
	const QVariant& get(const int row, const ColumnType& column) const;
	int getDataRowIndex(const int row) const;
	int getRowIndex(const int dataRowIndex) const;

	int getQuantity(const int dataRowIndex) const;
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
