#pragma once

#include "shortcuttype.h"
#include "magiccolumntype.h"
#include "filter.h"

#include <QString>
#include <QKeySequence>
#include <QVector>
#include <QScopedPointer>
#include <QMap>
#include <QFont>

class Settings
{
private:
	Settings();
	~Settings();

public:
	static Settings& instance();

	const QString& getAppDataDir() const;
	void setAppDataDir(const QString& appDataDir) const;
	const QString& getCollectionFile() const;
	const QString& getCollectionDb() const;
	const QString& getPricesDb() const;
	const QString& getPricesBz2File() const;
	const QString& getPricesJsonFile() const;
	const QString& getTagsDb() const;
	const QString& getCategoriesDb() const;
	const QString& getFiltersDir() const;
	const QString& getDecksDir() const;

	const QString& getPoolDataFile() const;
	void setPoolDataFile(const QString& poolDataFile);

	const QString& getCardImageDir() const;
	void setCardImageDir(const QString& cardImageDir);

	const QMap<ShortcutType, QKeySequence>& getShortcuts() const;
	void setShortcuts(const QMap<ShortcutType, QKeySequence>& shortcuts);

	const FilterNode::Ptr& getGlobalFilter() const;
	void setGlobalFilter(const FilterNode::Ptr& globalFilter);

	const QFont& getFont() const;
	void setFont(const QFont& font);

	bool getArtIsHighQuality() const;
	void setArtIsHighQuality(bool enabled);

	const QVector<mtg::ColumnType>& getCopyColumns() const;
	void setCopyColumns(const QVector<mtg::ColumnType>& copyColumns);

	bool getPoolViewButtons() const;
	void setPoolViewButtons(bool enabled);
	bool getCollectionViewButtons() const;
	void setCollectionViewButtons(bool enabled);
	bool getDeckViewButtons() const;
	void setDeckViewButtons(bool enabled);

private:

	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
