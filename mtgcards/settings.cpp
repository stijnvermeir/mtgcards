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

	Pimpl()
	{
		QSettings settings;
		setAppDataDir(settings.value("options/misc/appdatadir", QStandardPaths::writableLocation(QStandardPaths::DataLocation)).toString());
		poolDataFile_ = settings.value("options/datasources/allsetsjson").toString();
		cardImageDir_ = settings.value("options/datasources/cardpicturedir").toString();
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
