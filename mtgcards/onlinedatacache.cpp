#include "onlinedatacache.h"

#include "settings.h"

#include <QtSql>
#include <QVariant>
#include <QHash>
#include <QVector>
#include <QDebug>

using namespace mtg;

namespace {

const QVector<QPair<ColumnType, QString>> ONLINE_COLUMNS =
{
    { ColumnType::Price, "real"}
};

QVector<int> generateOnlineColumnIndices()
{
	QVector<int> indices(ColumnType::COUNT, -1);
	for (int i = 0; i < ONLINE_COLUMNS.size(); ++i)
	{
		indices[ONLINE_COLUMNS[i].first] = i;
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

QSqlDatabase conn()
{
	return QSqlDatabase::database("onlinedatacache");
}

QSqlError initDb()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "onlinedatacache");
	db.setDatabaseName(Settings::instance().getOnlineDataCacheDb());
	if (!db.open()) return db.lastError();

	if (!db.tables().contains("cache"))
	{
		QString query;
		QTextStream str(&query);
		str << "CREATE TABLE cache (id integer primary key, set_code text, name text";
		for (int i = 0; i < ONLINE_COLUMNS.size(); ++i)
		{
			str << ", " << (QString) ONLINE_COLUMNS[i].first << " " << ONLINE_COLUMNS[i].second;
		}
		str << ")";
		qDebug() << query;
		QSqlQuery q(db);
		if (!q.exec(query)) return q.lastError();
		if (!q.exec("CREATE UNIQUE INDEX cache_set_name_index ON cache (set_code, name)")) return q.lastError();
	}
	else
	{
		QSqlRecord record = db.record("cache");
		QSqlQuery q(db);
		for (int i = 0; i < ONLINE_COLUMNS.size(); ++i)
		{
			if (record.indexOf((QString) ONLINE_COLUMNS[i].first) == -1)
			{
				QString query;
				QTextStream str(&query);
				str << "ALTER TABLE cache ADD COLUMN " << (QString) ONLINE_COLUMNS[i].first << " " << ONLINE_COLUMNS[i].second;
				qDebug() << query;
				if (!q.exec(query)) return q.lastError();
			}
		}
	}

	return QSqlError();
}

} // namespace

struct OnlineDataCache::Pimpl
{
	QHash<QString, QVector<QVariant>> localCache_;

	Pimpl()
	{
		QSqlError err = initDb();
		if (err.isValid())
		{
			qDebug() << err;
		}

		QSqlDatabase db = conn();
		QSqlQuery q(db);
		q.exec("SELECT * FROM cache");
		QSqlRecord record = q.record();
		int sqlSetIndex = record.indexOf("set_code");
		int sqlNameIndex = record.indexOf("name");
		QVector<int> sqlColumnIndices(ONLINE_COLUMNS.size());
		for (int i = 0; i < ONLINE_COLUMNS.size(); ++i)
		{
			sqlColumnIndices[i] = record.indexOf((QString)ONLINE_COLUMNS[i].first);
		}
		while (q.next())
		{
			QVector<QVariant> entry(ONLINE_COLUMNS.size());
			for (int i = 0; i < ONLINE_COLUMNS.size(); ++i)
			{
				entry[i] = q.value(sqlColumnIndices[i]);
			}
			QString key = q.value(sqlSetIndex).toString() + q.value(sqlNameIndex).toString();
			localCache_[key] = entry;
		}
		q.finish();
	}

	QVariant get(const QString& set, const QString& name, const mtg::ColumnType& column) const
	{
		if (isOnlineColumn(column))
		{
			QString key = set + name;
			if (localCache_.contains(key))
			{
				return localCache_[key][onlineColumnToIndex(column)];
			}
		}
		return QVariant();
	}

	void set(const QString& set, const QString& name, const mtg::ColumnType& column, const QVariant& data)
	{
		if (isOnlineColumn(column))
		{
			// store in db
			QSqlDatabase db = conn();
			QSqlQuery q(db);
			q.prepare("SELECT id FROM cache WHERE set_code = ? AND name = ?");
			q.addBindValue(set);
			q.addBindValue(name);
			if (q.exec())
			{
				if (q.next())
				{
					int dbId = q.value(0).toInt();
					QString query;
					QTextStream str(&query);
					str << "UPDATE cache SET " << (QString)column << " = ? WHERE id = ?";
					q.prepare(query);
					q.addBindValue(data);
					q.addBindValue(dbId);
					q.exec();
				}
				else
				{
					QString query;
					QTextStream str(&query);
					str << "INSERT INTO cache(set_code, name, " << (QString)column << ") VALUES (?, ?, ?)";
					q.prepare(query);
					q.addBindValue(set);
					q.addBindValue(name);
					q.addBindValue(data);
					q.exec();
				}
			}
			if (q.lastError().isValid())
			{
				qDebug() << q.lastError();
			}

			// update local cache
			QString key = set + name;
			if (!localCache_.contains(key))
			{
				localCache_[key] = QVector<QVariant>(ONLINE_COLUMNS.size());
			}
			localCache_[key][onlineColumnToIndex(column)] = data;
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

QVariant OnlineDataCache::get(const QString& set, const QString& name, const mtg::ColumnType& column) const
{
	return pimpl_->get(set, name, column);
}

void OnlineDataCache::set(const QString& set, const QString& name, const mtg::ColumnType& column, const QVariant& data)
{
	pimpl_->set(set, name, column, data);
}

OnlineDataCache::OnlineDataCache()
	: pimpl_(new Pimpl())
{
}

OnlineDataCache::~OnlineDataCache()
{
}


