#pragma once

#include "magiccolumntype.h"
#include "magiclayouttype.h"

#include <QVariant>
#include <QStringList>

#include <memory>

namespace mtg {

class CardData
{
public:
	static CardData& instance();

	void reload();
	int getNumRows() const;
	const QVariant& get(const int row, const ColumnType& column) const;
	int findRow(const std::vector<std::pair<ColumnType, QVariant>>& criteria) const;
	std::pair<mtg::LayoutType, QStringList> getPictureFilenames(int row);

private:
	CardData();
	~CardData();

	struct Pimpl;
	std::unique_ptr<Pimpl> pimpl_;
};

} // namespace mtg
