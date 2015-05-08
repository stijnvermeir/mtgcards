#pragma once

#include "shortcuttype.h"

#include <QString>
#include <QKeySequence>

#include <map>
#include <memory>

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

	const std::map<ShortcutType, QKeySequence>& getShortcuts() const;
	void setShortcuts(const std::map<ShortcutType, QKeySequence>& shortcuts);
private:

	struct Pimpl;
	std::unique_ptr<Pimpl> pimpl_;
};
