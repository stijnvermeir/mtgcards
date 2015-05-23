#pragma once

#include "magiccolumntype.h"
#include "magiclayouttype.h"

#include <QVariant>
#include <QStringList>
#include <QScopedPointer>
#include <QPair>
#include <QVector>

namespace mtg {

class CardData
{
public:
	static CardData& instance();

	void reload();
	int getNumRows() const;
	const QVariant& get(const int row, const ColumnType& column) const;
	int findRow(const QVector<QPair<ColumnType, QVariant>>& criteria) const;
	int findRowFast(const QString& set, const QString& name, const QString& imageName = QString::null) const;
	QPair<mtg::LayoutType, QStringList> getPictureFilenames(int row);

private:
	CardData();
	~CardData();

	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};

} // namespace mtg
