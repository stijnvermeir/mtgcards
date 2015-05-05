#pragma once

#include "magiccolumntype.h"

#include <QVariant>

#include <memory>

namespace mtg {

class Collection
{
public:
	static Collection& instance();

	void reload();
	int getNumRows() const;
	const QVariant& get(const int row, const ColumnType& column) const;

private:
	Collection();
	~Collection();

	struct Pimpl;
	std::unique_ptr<Pimpl> pimpl_;
};

} // namespace mtg
