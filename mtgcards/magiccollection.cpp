#include "magiccollection.h"

#include "magiccarddata.h"
#include "deckmanager.h"
#include "settings.h"

#include <QtSql>
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
		if (!q.exec("CREATE TABLE card(id integer primary key, set_code varchar, name varchar, image_name varchar, quantity integer)")) return q.lastError();

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

		Row()
			: rowIndexInData(-1)
			, dbId(-1)
			, quantity(0)
		    , used(0) {}
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
		if (!q.exec("SELECT id, set_code, name, image_name, quantity FROM card")) return q.lastError();
		while (q.next())
		{
			auto set = q.value(1).toString();
			if (set == "NMS")
			{
				set = "NEM";
			}
			auto name = q.value(2).toString();
			auto imageName = q.value(3).toString();

			auto rowIndexInData = mtg::CardData::instance().findRowFast(set, name, imageName);
			if (rowIndexInData == -1)
			{
				rowIndexInData = mtg::CardData::instance().findRowFast(set, name);
				if (rowIndexInData == -1)
				{
					qDebug() << "Could not find" << set << name << "from collection";
				}
			}
			if (rowIndexInData != -1)
			{
				Row r;
				r.rowIndexInData = rowIndexInData;
				r.dbId = q.value(0).toInt();
				r.quantity = q.value(4).toInt();
				r.used = DeckManager::instance().getUsedCount(r.rowIndexInData);
				data_.push_back(r);
			}
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
            if (column == ColumnType::QuantityAll)
            {
                int total = 0;
                const auto& reprintRowIndicesInData = mtg::CardData::instance().findReprintRows(entry.rowIndexInData);
                for (const auto& i : reprintRowIndicesInData)
                {
                    auto rr = findRow(i);
                    if (rr)
                    {
                        total += rr->quantity.toInt();
                    }
                }
                return total;
            }
			if (column == ColumnType::Used)
			{
				return entry.used;
			}
            if (column == ColumnType::UsedAll)
            {
                return DeckManager::instance().getUsedAllCount(entry.rowIndexInData);
            }
			if (column == ColumnType::NotOwned)
			{
				auto notOwned = entry.used.toInt() - entry.quantity.toInt();
				return ((notOwned > 0) ? notOwned : 0);
			}
			return mtg::CardData::instance().get(entry.rowIndexInData, column);
		}
        if (column == ColumnType::Quantity || column == ColumnType::QuantityAll || column == ColumnType::Used || column == ColumnType::UsedAll)
        {
            return 0;
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

    int getQuantityAll(const int dataRowIndex) const
    {
        const auto& reprintRowIndicesInData = mtg::CardData::instance().findReprintRows(dataRowIndex);
        int total = 0;
        for (const auto& i : reprintRowIndicesInData)
        {
            total += getQuantity(i);
        }
        return total;
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
				q.prepare("INSERT INTO card(set_code, name, image_name, quantity) VALUES(?, ?, ?, ?)");
				q.addBindValue(setCode);
				q.addBindValue(name);
				q.addBindValue(imageName);
				q.addBindValue(newQuantity);
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

int Collection::getQuantityAll(const int dataRowIndex) const
{
    return pimpl_->getQuantityAll(dataRowIndex);
}

void Collection::setQuantity(const int dataRowIndex, const int newQuantity)
{
	pimpl_->setQuantity(dataRowIndex, newQuantity);
}

void Collection::setUsedCount(const int dataRowIndex, const int usedCount)
{
	pimpl_->setUsedCount(dataRowIndex, usedCount);
}
