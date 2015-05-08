#include "settings.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

using namespace std;

struct Settings::Pimpl
{
	QString appDataDir_;
	QString collectionFile_;
	QString filtersDir_;
	QString decksDir_;
	QString poolDataFile_;
	QString cardImageDir_;
	map<ShortcutType, QKeySequence> shortcuts_;

	Pimpl()
	{
		QSettings settings;
		setAppDataDir(settings.value("options/misc/appdatadir", QStandardPaths::writableLocation(QStandardPaths::DataLocation)).toString());
		poolDataFile_ = settings.value("options/datasources/allsetsjson").toString();
		cardImageDir_ = settings.value("options/datasources/cardpicturedir").toString();

		for (const ShortcutType& shortcut : ShortcutType::list())
		{
			QString key = "shortcuts/";
			key += QString(shortcut);
			if (settings.contains(key))
			{
				shortcuts_[shortcut] = QKeySequence(settings.value(key).toString());
			}
			else
			{
				shortcuts_[shortcut] = shortcut.getDefaultKeySequence();
			}
		}
	}

	void setAppDataDir(const QString& appDataDir)
	{
		QDir dir(appDataDir);
		dir.mkpath("filters");
		dir.mkpath("decks");
		QSettings settings;
		settings.setValue("options/misc/appdatadir", appDataDir);
		appDataDir_ = appDataDir;
		collectionFile_ = appDataDir_ + QDir::separator() + "collection.json";
		filtersDir_ = appDataDir_ + QDir::separator() + "filters";
		decksDir_ = appDataDir_ + QDir::separator() + "decks";
	}

	void setPoolDataFile(const QString& poolDataFile)
	{
		poolDataFile_ = poolDataFile;
		QSettings settings;
		settings.setValue("options/datasources/allsetsjson", poolDataFile_);
	}

	void setCardImageDir(const QString& cardImageDir)
	{
		cardImageDir_ = cardImageDir;
		QSettings settings;
		settings.setValue("options/datasources/cardpicturedir", cardImageDir_);
	}

	void setShortcuts(const map<ShortcutType, QKeySequence>& shortcuts)
	{
		shortcuts_ = shortcuts;

		QSettings settings;
		for (const auto& entry : shortcuts_)
		{
			QString key = "shortcuts/";
			key += QString(entry.first);
			settings.setValue(key, entry.second.toString(QKeySequence::NativeText));
		}
	}
};

Settings::Settings()
	: pimpl_(new Pimpl())
{
}

Settings::~Settings()
{
}

Settings& Settings::instance()
{
	static Settings inst;
	return inst;
}

const QString& Settings::getAppDataDir() const
{
	return pimpl_->appDataDir_;
}

void Settings::setAppDataDir(const QString& appDataDir) const
{
	pimpl_->setAppDataDir(appDataDir);
}

const QString& Settings::getCollectionFile() const
{
	return pimpl_->collectionFile_;
}

const QString& Settings::getFiltersDir() const
{
	return pimpl_->filtersDir_;
}

const QString& Settings::getDecksDir() const
{
	return pimpl_->decksDir_;
}

const QString& Settings::getPoolDataFile() const
{
	return pimpl_->poolDataFile_;
}

void Settings::setPoolDataFile(const QString& poolDataFile)
{
	pimpl_->setPoolDataFile(poolDataFile);
}

const QString& Settings::getCardImageDir() const
{
	return pimpl_->cardImageDir_;
}

void Settings::setCardImageDir(const QString& cardImageDir)
{
	pimpl_->setCardImageDir(cardImageDir);
}

const map<ShortcutType, QKeySequence>& Settings::getShortcuts() const
{
	return pimpl_->shortcuts_;
}

void Settings::setShortcuts(const map<ShortcutType, QKeySequence>& shortcuts)
{
	pimpl_->setShortcuts(shortcuts);
}
