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
    QString collectionDb_;
	QString pricesDb_;
	QString pricesBz2_;
	QString pricesJson_;
	QString tagsDb_;
	QString filtersDir_;
	QString decksDir_;
	QString poolDataFile_;
	QString cardImageDir_;
	bool ultraHighQualityArt_;
	QMap<ShortcutType, QKeySequence> shortcuts_;
	FilterNode::Ptr globalFilter_;
	QFont font_;
	QVector<mtg::ColumnType> copyColumns_;

	Pimpl()
	{
		QSettings settings;
		setAppDataDir(settings.value("options/misc/appdatadir", QStandardPaths::writableLocation(QStandardPaths::DataLocation)).toString());
		poolDataFile_ = settings.value("options/datasources/allprintingssqlite", appDataDir_ + QDir::separator() + "AllPrintings.sqlite").toString();
		cardImageDir_ = settings.value("options/datasources/cardpicturedir", appDataDir_ + QDir::separator() + "cardart").toString();
		ultraHighQualityArt_ = settings.value("options/misc/ultraHighQualityArtEnabled", false).toBool();

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

		if (settings.contains("globalfilter"))
		{
			globalFilter_ = FilterNode::createFromJson(QJsonDocument::fromJson(settings.value("globalfilter").toString().toUtf8()));
		}

		if (settings.contains("options/font"))
		{
			font_.fromString(settings.value("options/font").toString());
		}

		if (settings.contains("options/copyColumns"))
		{
			QStringList copyColumns = settings.value("options/copyColumns").toStringList();
			for (const QString& copyColumn : copyColumns)
			{
				auto column = mtg::ColumnType(copyColumn);
				if (column != mtg::ColumnType::UNKNOWN && column < mtg::ColumnType::COUNT)
				{
					copyColumns_.push_back(column);
				}
			}
		}
		else
		{
			copyColumns_.push_back(mtg::ColumnType::SetCode);
			copyColumns_.push_back(mtg::ColumnType::Name);
			copyColumns_.push_back(mtg::ColumnType::Quantity);
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
        collectionDb_ = appDataDir_ + QDir::separator() + "collection.db";
		pricesDb_ = appDataDir_ + QDir::separator() + "prices.db";
		pricesBz2_ = appDataDir_ + QDir::separator() + "AllPrices.json.bz2";
		pricesJson_ = appDataDir_ + QDir::separator() + "AllPrices.json";
		tagsDb_ =  appDataDir_ + QDir::separator() + "tags.db";
		filtersDir_ = appDataDir_ + QDir::separator() + "filters";
		decksDir_ = appDataDir_ + QDir::separator() + "decks";
	}

	void setPoolDataFile(const QString& poolDataFile)
	{
		poolDataFile_ = poolDataFile;
		QSettings settings;
		settings.setValue("options/datasources/allprintingssqlite", poolDataFile_);
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

	void setFont(const QFont& font)
	{
		font_ = font;

		QSettings settings;
		settings.setValue("options/font", font_.toString());
	}

	void setUltraHighQualityArtEnabled(bool enabled)
	{
		ultraHighQualityArt_ = enabled;

		QSettings settings;
		settings.setValue("options/misc/ultraHighQualityArtEnabled", ultraHighQualityArt_);
	}

	void setCopyColumns(const QVector<mtg::ColumnType>& copyColumns)
	{
		copyColumns_ = copyColumns;

		QStringList value;
		for (const mtg::ColumnType& copyColumn : copyColumns_)
		{
			value << (QString) copyColumn;
		}

		QSettings settings;
		settings.setValue("options/copyColumns", value);
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

const QString& Settings::getCollectionDb() const
{
    return pimpl_->collectionDb_;
}

const QString& Settings::getPricesDb() const
{
	return pimpl_->pricesDb_;
}

const QString& Settings::getPricesBz2File() const
{
	return pimpl_->pricesBz2_;
}

const QString& Settings::getPricesJsonFile() const
{
	return pimpl_->pricesJson_;
}

const QString& Settings::getTagsDb() const
{
	return pimpl_->tagsDb_;
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

const FilterNode::Ptr& Settings::getGlobalFilter() const
{
	return pimpl_->globalFilter_;
}

void Settings::setGlobalFilter(const FilterNode::Ptr& globalFilter)
{
	pimpl_->setGlobalFilter(globalFilter);
}

const QFont& Settings::getFont() const
{
	return pimpl_->font_;
}

void Settings::setFont(const QFont& font)
{
	pimpl_->setFont(font);
}

bool Settings::getArtIsHighQuality() const
{
	return pimpl_->ultraHighQualityArt_;
}

void Settings::setArtIsHighQuality(bool enabled)
{
	pimpl_->setUltraHighQualityArtEnabled(enabled);
}

const QVector<mtg::ColumnType>& Settings::getCopyColumns() const
{
	return pimpl_->copyColumns_;
}

void Settings::setCopyColumns(const QVector<mtg::ColumnType>& copyColumns)
{
	pimpl_->setCopyColumns(copyColumns);
}
