#pragma once

#include "shortcuttype.h"
#include "usercolumn.h"
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

	struct Mkm
	{
		QString appToken;
		QString appSecret;
		QString accessToken;
		QString accessTokenSecret;
		bool useSandbox;

		const QString& getEndpoint() const;
	};

	static Settings& instance();

	const QString& getAppDataDir() const;
	void setAppDataDir(const QString& appDataDir) const;
	const QString& getCollectionFile() const;
	const QString& getCollectionDb() const;
	const QString& getOnlineDataCacheDb() const;
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

	const FilterNode::Ptr& getGlobalFilter() const;
	void setGlobalFilter(const FilterNode::Ptr& globalFilter);

	const QFont& getFont() const;
	void setFont(const QFont& font);

	const Mkm& getMkm() const;
	void setMkm(const Mkm& mkm);
private:

	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
