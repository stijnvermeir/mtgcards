#include "categories.h"
#include "settings.h"

#include <QtSql>
#include <QHash>
#include <QList>
#include <QDebug>

namespace {

QSqlDatabase conn()
{
	return QSqlDatabase::database("categories");
}

QSqlError initDb()
{
	const int DB_VERSION = 1;
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "categories");
	db.setDatabaseName(Settings::instance().getCategoriesDb());
	if (!db.open()) return db.lastError();

	if (!db.tables().contains("version")) // database does not exist yet
	{
		qDebug() << "Creating categories db ...";
		QSqlQuery q(db);

		// version table
		if (!q.exec("CREATE TABLE version(number integer primary key)")) return q.lastError();
		if (!q.prepare("INSERT INTO version(number) VALUES (?)")) return q.lastError();
		q.addBindValue(DB_VERSION);
		if (!q.exec()) return q.lastError();

		// category table
		if (!q.exec("CREATE TABLE category(id integer primary key, name text)")) return q.lastError();

		qDebug() << "Categories db created succesfully.";
	}
	else
	{
		QSqlQuery q(db);
		if (!q.exec("SELECT max(number) FROM version")) return q.lastError();
		if (!q.next()) return q.lastError();
		int currentVersion = q.value(0).toInt();
		if (currentVersion != DB_VERSION)
		{
			qDebug() << "Categories db version" << currentVersion;
			// for auto database model updates later
			qFatal("Invalid categories db version");
		}
	}

	return QSqlError();
}

} // namespace

struct Categories::Pimpl
{
	QHash<QString, int> categories_;

	Pimpl()
	{
		QSqlError err = initDb();
		if (err.isValid())
		{
			qDebug() << err;
		}
		err = load();
		if (err.isValid())
		{
			qDebug() << err;
		}
	}

	QSqlError load()
	{
		categories_.clear();

		QSqlDatabase db = conn();
		QSqlQuery q(db);
		if (!q.exec("SELECT id, name FROM category")) return q.lastError();
		while (q.next())
		{
			auto id = q.value(0).toInt();
			auto name = q.value(1).toString();
			categories_[name] = id;
		}
		return QSqlError();
	}

	QStringList getCategories() const
	{
		auto categories = categories_.keys();
		categories.sort(Qt::CaseInsensitive);
		return categories;
	}

	void addCategory(const QString& category)
	{
		if (!categories_.contains(category))
		{
			QSqlDatabase db = conn();
			QSqlQuery q(db);
			if (!q.prepare("INSERT INTO category(name) VALUES(?)")) return;
			q.addBindValue(category);
			if (!q.exec()) return;
			int categoryId = q.lastInsertId().toInt();
			categories_[category] = categoryId;
		}
	}

	void removeCategory(const QString& category)
	{
		if (categories_.contains(category))
		{
			int categoryId = categories_[category];
			QSqlDatabase db = conn();
			QSqlQuery q(db);
			if (!q.prepare("DELETE FROM category WHERE id = ?")) return;
			q.addBindValue(categoryId);
			if (!q.exec()) return;
			categories_.remove(category);
		}
	}
};

Categories& Categories::instance()
{
	static Categories inst;
	return inst;
}

QStringList Categories::getCategories() const
{
	return pimpl_->getCategories();
}

void Categories::addCategory(const QString &category)
{
	pimpl_->addCategory(category);
}

void Categories::removeCategory(const QString& category)
{
	pimpl_->removeCategory(category);
}

Categories::Categories()
    : pimpl_(new Pimpl())
{
}

Categories::~Categories()
{
}
