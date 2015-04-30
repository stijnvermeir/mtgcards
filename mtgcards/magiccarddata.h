#pragma once

#include "magiccolumntype.h"

#include <QVariant>

#include <memory>

namespace mtg {

class CardData
{
public:
	static CardData& instance();
	~CardData();

	void reload();
	int getNumRows() const;
	const QVariant& get(const int row, const ColumnType& column) const;
	int findRow(const std::vector<std::pair<ColumnType, QVariant>>& criteria) const;

private:
	CardData();

	struct Pimpl;
	std::unique_ptr<Pimpl> pimpl_;
};

} // namespace mtg
