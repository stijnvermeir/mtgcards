#pragma once

#include "magiccolumntype.h"
#include "magiclayouttype.h"
#include "magicruling.h"

#include <QVariant>
#include <QStringList>
#include <QScopedPointer>
#include <QPair>
#include <QVector>

namespace mtg {

class CardData
{
public:
	struct PictureInfo
	{
		mtg::LayoutType layout;
		QStringList filenames;
		QStringList missing;
	};

	static CardData& instance();

	void reload();
	int getNumRows() const;
	QVariant get(const int row, const ColumnType& column) const;
	int findRow(const QVector<QPair<ColumnType, QVariant>>& criteria) const;
	int findRowFast(const QString& set, const QString& name, const QString& imageName = QString::null) const;
	PictureInfo getPictureInfo(int row);
	const QVector<mtg::Ruling>& getRulings(int row);
	void fetchOnlineData(const int row);

private:
	CardData();
	~CardData();

	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};

} // namespace mtg
