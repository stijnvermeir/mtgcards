#include "settings.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QVariantMap>
#include <QVariantList>
#include <QJsonDocument>

struct Settings::Pimpl
{
	QString appDataDir_;
	QString collectionFile_;
	QString filtersDir_;
	QString decksDir_;
	QString poolDataFile_;
	QString cardImageDir_;
	QMap<ShortcutType, QKeySequence> shortcuts_;
	QVector<UserColumn> userColumns_;
	FilterNode::Ptr globalFilter_;

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

		if (settings.contains("options/usercolumns"))
		{
			QVariantList userColumnList = QJsonDocument::fromJson(settings.value("options/usercolumns").toString().toUtf8()).toVariant().toList();
			for (int i = 0; i < userColumnList.size(); ++i)
			{
				QVariantMap userColumnMap = userColumnList[i].toMap();
				UserColumn userColumn;
				userColumn.dataType_ = UserColumn::DataType(userColumnMap["dataType"].toString());
				userColumn.name_ = userColumnMap["name"].toString();
				userColumns_.push_back(userColumn);
			}
		}

		if (settings.contains("globalfilter"))
		{
			globalFilter_ = FilterNode::createFromJson(QJsonDocument::fromJson(settings.value("globalfilter").toString().toUtf8()));
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

	void setShortcuts(const QMap<ShortcutType, QKeySequence>& shortcuts)
	{
		shortcuts_ = shortcuts;

		QSettings settings;
		for (auto it = shortcuts_.cbegin(); it != shortcuts_.cend(); ++it)
		{
			QString key = "shortcuts/";
			key += QString(it.key());
			settings.setValue(key, it.value().toString(QKeySequence::NativeText));
		}
	}

	void setUserColumns(const QVector<UserColumn>& userColumns)
	{
		userColumns_ = userColumns;

		QSettings settings;
		if (userColumns_.empty())
		{
			settings.remove("options/usercolumns");
			return;
		}
		QVariantList userColumnList;
		for (const UserColumn& entry : userColumns_)
		{
			QVariantMap userColumnMap;
			userColumnMap["dataType"] = static_cast<QString>(entry.dataType_);
			userColumnMap["name"] = entry.name_;
			userColumnList.push_back(userColumnMap);
		}
		settings.setValue("options/usercolumns", QString(QJsonDocument::fromVariant(userColumnList).toJson()));
	}

	void setGlobalFilter(const FilterNode::Ptr& globalFilter)
	{
		globalFilter_ = globalFilter;

		QSettings settings;
		if (globalFilter_)
		{
			settings.setValue("globalfilter", QString(globalFilter_->toJson().toJson(QJsonDocument::Compact)));
		}
		else
		{
			settings.remove("globalfilter");
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

const QMap<ShortcutType, QKeySequence>& Settings::getShortcuts() const
{
	return pimpl_->shortcuts_;
}

void Settings::setShortcuts(const QMap<ShortcutType, QKeySequence>& shortcuts)
{
	pimpl_->setShortcuts(shortcuts);
}

const QVector<UserColumn>& Settings::getUserColumns() const
{
	return pimpl_->userColumns_;
}

void Settings::setUserColumns(const QVector<UserColumn>& userColumns)
{
	pimpl_->setUserColumns(userColumns);
}

const FilterNode::Ptr& Settings::getGlobalFilter() const
{
	return pimpl_->globalFilter_;
}

void Settings::setGlobalFilter(const FilterNode::Ptr& globalFilter)
{
	pimpl_->setGlobalFilter(globalFilter);
}
