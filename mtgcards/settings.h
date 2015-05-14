#pragma once

#include "shortcuttype.h"
#include "usercolumn.h"

#include <QString>
#include <QKeySequence>
#include <QVector>
#include <QScopedPointer>
#include <QMap>

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
	const QString& getFiltersDir() const;
	const QString& getDecksDir() const;

	const QString& getPoolDataFile() const;
	void setPoolDataFile(const QString& poolDataFile);

	const QString& getCardImageDir() const;
	void setCardImageDir(const QString& cardImageDir);

	const QMap<ShortcutType, QKeySequence>& getShortcuts() const;
	void setShortcuts(const QMap<ShortcutType, QKeySequence>& shortcuts);

	const QVector<UserColumn>& getUserColumns() const;
	void setUserColumns(const QVector<UserColumn>& userColumns);
private:

	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
