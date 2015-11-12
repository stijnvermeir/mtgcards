#include "magiccollection.h"

#include "magiccarddata.h"
#include "deckmanager.h"
#include "settings.h"

#include <QtSql>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QDebug>

using namespace std;
using namespace mtg;

namespace {

QSqlDatabase conn()
{
	return QSqlDatabase::database("collection");
}

QSqlError initDb()
{
	const int DB_VERSION = 1;
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "collection");
	db.setDatabaseName(Settings::instance().getCollectionDb());
	if (!db.open()) return db.lastError();

	if (!db.tables().contains("version")) // database does not exist yet
	{
		qDebug() << "Creating collection db ...";
		QSqlQuery q(db);

		// version table
		if (!q.exec("CREATE TABLE version(number integer primary key)")) return q.lastError();
		if (!q.prepare("INSERT INTO version(number) VALUES (?)")) return q.lastError();
		q.addBindValue(DB_VERSION);
		if (!q.exec()) return q.lastError();

		// card table
		if (!q.exec("CREATE TABLE card(id integer primary key, set_code varchar, name varchar, image_name varchar, quantity integer, user_data varchar)")) return q.lastError();
		QFile file(Settings::instance().getCollectionFile());
		if (file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			qDebug() << "Converting collection.json to collection.db ...";
			QJsonDocument d = QJsonDocument::fromJson(QString(file.readAll()).toUtf8());
			QJsonObject obj = d.object();
			QJsonArray cards = obj["cards"].toArray();
			if (!db.transaction()) return db.lastError();
			for (const auto& c : cards)
			{
				QJsonObject card = c.toObject();
				auto set = card["Set"].toString();
				auto name = card["Name"].toString();
				auto imageName = card["ImageName"].toString();
				auto quantity = card["Quantity"].toInt();
				QVariantMap userData = UserColumn::loadFromJson(card);
				if (!q.prepare("INSERT INTO card(set_code, name, image_name, quantity, user_data) VALUES (?, ?, ?, ?, ?)"))
				{
					db.rollback();
					return q.lastError();
				}
				q.addBindValue(set);
				q.addBindValue(name);
				q.addBindValue(imageName);
				q.addBindValue(quantity);
				QJsonObject userDataObj;
				UserColumn::saveToJson(userDataObj, userData);
				QJsonDocument userDataDoc(userDataObj);
				q.addBindValue(userDataDoc.toJson(QJsonDocument::Compact));
				if (!q.exec())
				{
					db.rollback();
					return q.lastError();
				}
			}
			if (!db.commit()) return db.lastError();
			qDebug() << "Successfully converted collection.json to db.";
		}

		qDebug() << "Collection db created succesfully.";
	}
	else
	{
		QSqlQuery q(db);
		if (!q.exec("SELECT max(number) FROM version")) return q.lastError();
		if (!q.next()) return q.lastError();
		int currentVersion = q.value(0).toInt();
		if (currentVersion != DB_VERSION)
		{
			qDebug() << "Collection db version" << currentVersion;
			// for auto database model updates later
			qFatal("Invalid collection db version");
		}
	}

	return QSqlError();
}

} // namespace

struct Collection::Pimpl
{
	struct Row
	{
		int rowIndexInData;
		int dbId;
		QVariant quantity;
		QVariant used;
		QVariantMap userData;

		Row()
			: rowIndexInData(-1)
			, dbId(-1)
			, quantity(0)
			, used(0)
			, userData() {}
	};
	QVector<Row> data_;

	Pimpl()
		: data_()
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
		data_.clear();

		QSqlDatabase db = conn();
		QSqlQuery q(db);
		if (!q.exec("SELECT id, set_code, name, image_name, quantity, user_data FROM card")) return q.lastError();
		while (q.next())
		{
			auto set = q.value(1).toString();
			auto name = q.value(2).toString();
			auto imageName = q.value(3).toString();

			Row r;
			r.rowIndexInData = mtg::CardData::instance().findRowFast(set, name, imageName);
			r.dbId = q.value(0).toInt();
			r.quantity = q.value(4).toInt();
			r.used = DeckManager::instance().getUsedCount(r.rowIndexInData);
			QJsonDocument userDataDoc = QJsonDocument::fromJson(q.value(5).toByteArray());
			r.userData = UserColumn::loadFromJson(userDataDoc.object());
			data_.push_back(r);
		}

		return QSqlError();
	}

	int getNumRows() const
	{
		return data_.size();
	}

	int getNumCards() const
	{
		int numCards = 0;
		for (const auto& row : data_)
		{
			numCards += row.quantity.toInt();
		}
		return numCards;
	}

	QVariant get(const int row, const ColumnType& column) const
	{
		if (row >= 0 && row < getNumRows())
		{
			const Row& entry = data_[row];
			if (column == ColumnType::Quantity)
			{
				return entry.quantity;
			}
			if (column == ColumnType::Used)
			{
				return entry.used;
			}
			if (column == ColumnType::UserDefined)
			{
				auto it = entry.userData.find(column.userColumn().name_);
				if (it != entry.userData.cend())
				{
					return it.value();
				}
				return column.userColumn().dataType_.getEmptyVariant();
			}
			return mtg::CardData::instance().get(entry.rowIndexInData, column);
		}
		return QVariant();
	}

	int getDataRowIndex(const int row) const
	{
		if (row >= 0 && row < getNumRows())
		{
			return data_[row].rowIndexInData;
		}
		return -1;
	}

	const Row* findRow(const int dataRowIndex) const
	{
		auto it = find_if(data_.begin(), data_.end(), [&dataRowIndex](const Row& row)
			{ return row.rowIndexInData == dataRowIndex; });

		if (it != data_.end())
		{
			return it;
		}
		return nullptr;
	}

	Row* findRow(const int dataRowIndex)
	{
		auto it = find_if(data_.begin(), data_.end(), [&dataRowIndex](const Row& row)
			{ return row.rowIndexInData == dataRowIndex; });

		if (it != data_.end())
		{
			return it;
		}
		return nullptr;
	}

	int getRowIndex(const int dataRowIndex) const
	{
		auto row = findRow(dataRowIndex);
		if (row)
		{
			return row - data_.begin();
		}
		return -1;
	}

	int getQuantity(const int dataRowIndex) const
	{
		auto row = findRow(dataRowIndex);
		if (row)
		{
			return row->quantity.toInt();
		}
		return 0;
	}

	void setQuantity(const int dataRowIndex, const int newQuantity)
	{
		auto row = findRow(dataRowIndex);
		if (row)
		{
			if (newQuantity >= 0)
			{
				QSqlDatabase db = conn();
				QSqlQuery q(db);
				q.prepare("UPDATE card SET quantity = ? WHERE id = ?");
				q.addBindValue(newQuantity);
				q.addBindValue(row->dbId);
				q.exec();
				if (!q.lastError().isValid())
				{
					row->quantity = newQuantity;
				}
			}
			else
			{
				QSqlDatabase db = conn();
				QSqlQuery q(db);
				q.prepare("DELETE FROM card WHERE id = ?");
				q.addBindValue(row->dbId);
				q.exec();
				if (!q.lastError().isValid())
				{
					data_.erase(row);
				}
			}
		}
		else
		{
			if (newQuantity >= 0)
			{
				auto setCode = mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::SetCode).toString();
				auto name = mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::Name).toString();
				auto imageName = mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::ImageName).toString();

				QSqlDatabase db = conn();
				QSqlQuery q(db);
				q.prepare("INSERT INTO card(set_code, name, image_name, quantity, user_data) VALUES(?, ?, ?, ?, ?)");
				q.addBindValue(setCode);
				q.addBindValue(name);
				q.addBindValue(imageName);
				q.addBindValue(newQuantity);
				q.addBindValue(QByteArray("{}"));
				q.exec();
				QVariant dbId = q.lastInsertId();
				if (dbId.isValid() && !q.lastError().isValid())
				{
					Row newRow;
					newRow.rowIndexInData = dataRowIndex;
					newRow.dbId = dbId.toInt();
					newRow.quantity = newQuantity;
					newRow.used = DeckManager::instance().getUsedCount(newRow.rowIndexInData);
					data_.push_back(newRow);
				}
			}
		}
	}

	void setUsedCount(const int dataRowIndex, const int usedCount)
	{
		auto row = findRow(dataRowIndex);
		if (row)
		{
			row->used = usedCount;
		}
	}

	void set(const int row, const ColumnType& column, const QVariant& data)
	{
		if (row >= 0 && row < getNumRows())
		{
			Row& entry = data_[row];
			if (column == ColumnType::UserDefined)
			{
				QVariantMap userDataCopy = entry.userData;
				userDataCopy[column.userColumn().name_] = data;
				QJsonObject obj;
				UserColumn::saveToJson(obj, userDataCopy);
				QJsonDocument doc(obj);

				QSqlDatabase db = conn();
				QSqlQuery q(db);
				q.prepare("UPDATE card SET user_data = ? WHERE id = ?");
				q.addBindValue(doc.toJson(QJsonDocument::Compact));
				q.addBindValue(entry.dbId);
				q.exec();
				if (!q.lastError().isValid())
				{
					entry.userData = userDataCopy;
				}
			}
		}
	}
};

Collection& Collection::instance()
{
	static Collection inst;
	return inst;
}

Collection::Collection()
	: pimpl_(new Pimpl())
{
}

Collection::~Collection()
{
}

QSqlDatabase Collection::getConnection()
{
	return conn();
}

void Collection::load()
{
	QSqlError err = pimpl_->load();
	if (err.isValid())
	{
		qDebug() << err;
	}
}

int Collection::getNumRows() const
{
	return pimpl_->getNumRows();
}

int Collection::getNumCards() const
{
	return pimpl_->getNumCards();
}

QVariant Collection::get(const int row, const ColumnType& column) const
{
	return pimpl_->get(row, column);
}

int Collection::getDataRowIndex(const int row) const
{
	return pimpl_->getDataRowIndex(row);
}

int Collection::getRowIndex(const int dataRowIndex) const
{
	return pimpl_->getRowIndex(dataRowIndex);
}

int Collection::getQuantity(const int dataRowIndex) const
{
	return pimpl_->getQuantity(dataRowIndex);
}

void Collection::setQuantity(const int dataRowIndex, const int newQuantity)
{
	pimpl_->setQuantity(dataRowIndex, newQuantity);
}

void Collection::setUsedCount(const int dataRowIndex, const int usedCount)
{
	pimpl_->setUsedCount(dataRowIndex, usedCount);
}

void Collection::set(const int row, const ColumnType& column, const QVariant& data)
{
	pimpl_->set(row, column, data);
}
