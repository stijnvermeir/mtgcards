#pragma once

#include "magiccolumntype.h"

#include <QVariant>
#include <memory>

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

private:
	Collection();
	~Collection();

	struct Pimpl;
	std::unique_ptr<Pimpl> pimpl_;
};

} // namespace mtg
