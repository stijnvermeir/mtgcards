#include "onlinedatacache.h"

#include "settings.h"

#include <QHash>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

using namespace mtg;

namespace {

const QVector<ColumnType> ONLINE_COLUMNS =
{
	ColumnType::PriceLowest,
	ColumnType::PriceLowestFoil,
	ColumnType::PriceAverage,
	ColumnType::PriceTrend,
	ColumnType::MkmProductId,
	ColumnType::MkmMetaproductId
};

QVector<int> generateOnlineColumnIndices()
{
	QVector<int> indices(ColumnType::COUNT, -1);
	for (int i = 0; i < ONLINE_COLUMNS.size(); ++i)
	{
		indices[ONLINE_COLUMNS[i]] = i;
	}
	return indices;
}

int onlineColumnToIndex(const ColumnType::type_t column)
{
	static const QVector<int> ONLINE_COLUMN_INDICES = generateOnlineColumnIndices();
	return ONLINE_COLUMN_INDICES[column];
}

bool isOnlineColumn(const ColumnType::type_t column)
{
	return (onlineColumnToIndex(column) >= 0);
}

} // namespace

struct OnlineDataCache::Pimpl
{
	QHash<QString, QVector<QVariant>> cache_;

	Pimpl()
	{
		QFile file(Settings::instance().getOnlineDataCacheFile());
		if (file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QJsonDocument d = QJsonDocument::fromJson(QString(file.readAll()).toUtf8());
			QJsonObject cache = d.object();
			for (QJsonObject::iterator it = cache.begin(), end = cache.end(); it != end; ++it)
			{
				QVector<QVariant>& item = cache_[it.key()];
				item.fill(QVariant(), ONLINE_COLUMNS.size());
				QJsonObject entry = it.value().toObject();
				for (QJsonObject::iterator i = entry.begin(), e = entry.end(); i != e; ++i)
				{
					ColumnType column(i.key());
					if (isOnlineColumn(column))
					{
						item[onlineColumnToIndex(column)] = ((QJsonValue) i.value()).toVariant();
					}
				}
			}
		}
	}

	const QVariant& get(const QString& set, const QString& name, const mtg::ColumnType& column) const
	{
		if (isOnlineColumn(column))
		{
			QString key = set + " " + name;
			if (cache_.contains(key))
			{
				return cache_[key][onlineColumnToIndex(column)];
			}
		}
		static const QVariant EMPTY;
		return EMPTY;
	}

	void set(const QString& set, const QString& name, const mtg::ColumnType& column, const QVariant& data)
	{
		if (isOnlineColumn(column))
		{
			QString key = set + " " + name;
			if (!cache_.contains(key))
			{
				cache_[key].fill(QVariant(), ONLINE_COLUMNS.size());
			}
			cache_[key][onlineColumnToIndex(column)] = data;
		}
	}

	void save()
	{
		QJsonObject cache;
		for (QHash<QString, QVector<QVariant>>::iterator it = cache_.begin(), end = cache_.end(); it != end; ++it)
		{
			QJsonObject entry;
			for (int column = 0; column < ONLINE_COLUMNS.size(); ++column)
			{
				entry[(QString) ONLINE_COLUMNS[column]] = QJsonValue::fromVariant(it.value()[column]);
			}
			cache[it.key()] = entry;
		}
		QJsonDocument doc(cache);
		QFile file(Settings::instance().getOnlineDataCacheFile());
		if (!file.open(QIODevice::WriteOnly))
		{
			qWarning() << "Failed to save to file " << file.fileName();
		}
		else
		{
			file.write(doc.toJson());
		}
	}
};

bool OnlineDataCache::isOnlineColumn(const mtg::ColumnType::type_t column)
{
	return ::isOnlineColumn(column);
}

OnlineDataCache& OnlineDataCache::instance()
{
	static OnlineDataCache inst;
	return inst;
}

const QVariant& OnlineDataCache::get(const QString& set, const QString& name, const mtg::ColumnType& column) const
{
	return pimpl_->get(set, name, column);
}

void OnlineDataCache::set(const QString& set, const QString& name, const mtg::ColumnType& column, const QVariant& data)
{
	pimpl_->set(set, name, column, data);
}

void OnlineDataCache::save()
{
	pimpl_->save();
}

OnlineDataCache::OnlineDataCache()
	: pimpl_(new Pimpl())
{
}

OnlineDataCache::~OnlineDataCache()
{
}


