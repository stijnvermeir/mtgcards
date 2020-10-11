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

	int getNumRows() const;
	QVariant get(const int row, const ColumnType& column) const;
	int findRow(const QVector<QPair<ColumnType, QVariant>>& criteria) const;
	int findRowFast(const QString& set, const QString& name, const QString& imageName = QString()) const;
	int findRowFastByUuid(const QString& uuid) const;
    const QVector<int>& findRowsFast(const QString& name) const;
    const QVector<int>& findReprintRows(const int row) const;
	PictureInfo getPictureInfo(int row, bool hq = false, bool doDownload = false);
	const QVector<mtg::Ruling>& getRulings(int row);
	void fetchOnlineData(const int row);
	QStringList getCardTagCompletions(const int row);
	void updateTags(const int row, const QString& update);

private:
	CardData();
	~CardData();

	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};

} // namespace mtg
