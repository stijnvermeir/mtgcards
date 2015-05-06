#include "magiccollection.h"

#include "magiccarddata.h"
#include "settings.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include <array>
#include <vector>

using namespace std;
using namespace mtg;

struct Collection::Pimpl
{
	struct Row
	{
		int rowIndexInData;
		QVariant quantity;
		QVariant used;

		Row()
			: rowIndexInData(-1)
			, quantity(0)
			, used(0) {}
	};
	vector<Row> data_;

	Pimpl()
		: data_()
	{
		load();
	}

	void load()
	{
		data_.clear();

		QFile file(Settings::instance().getCollectionFile());
		if (file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QJsonDocument d = QJsonDocument::fromJson(QString(file.readAll()).toUtf8());
			QJsonObject obj = d.object();
			QJsonArray cards = obj["cards"].toArray();
			data_.reserve(cards.size());
			for (const auto& c : cards)
			{
				QJsonObject card = c.toObject();
				auto set = card["Set"].toString();
				auto name = card["Name"].toString();
				vector<pair<ColumnType, QVariant>> criteria;
				criteria.emplace_back(ColumnType::SetCode, set);
				criteria.emplace_back(ColumnType::Name, name);
				Row r;
				r.rowIndexInData = mtg::CardData::instance().findRow(criteria);
				r.quantity = card["Quantity"].toInt();
				r.used = 0; // TODO
				data_.push_back(r);
			}
		}
	}

	void save()
	{
		QJsonArray cards;
		for (const Row& r : data_)
		{
			QJsonObject cardObj;
			cardObj["Set"] = mtg::CardData::instance().get(r.rowIndexInData, ColumnType::SetCode).toString();
			cardObj["Name"] = mtg::CardData::instance().get(r.rowIndexInData, ColumnType::Name).toString();
			cardObj["Quantity"] = r.quantity.toInt();
			cards.append(cardObj);
		}
		QJsonObject obj;
		obj["cards"] = cards;
		QJsonDocument doc(obj);
		QFile file(Settings::instance().getCollectionFile());
		if (!file.open(QIODevice::WriteOnly))
		{
			qWarning() << "Failed to save to file " << file.fileName();
		}
		else
		{
			file.write(doc.toJson());
		}
	}

	int getNumRows() const
	{
		return static_cast<int>(data_.size());
	}

	const QVariant& get(const int row, const ColumnType& column) const
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
			return mtg::CardData::instance().get(entry.rowIndexInData, column);
		}
		static const QVariant EMPTY;
		return EMPTY;
	}

	int getDataRowIndex(const int row) const
	{
		if (row >= 0 && row < getNumRows())
		{
			return data_[row].rowIndexInData;
		}
		return -1;
	}

	int getRowIndex(const int dataRowIndex) const
	{
		auto it = find_if(data_.begin(), data_.end(), [&dataRowIndex](const Row& row) { return row.rowIndexInData == dataRowIndex; });
		if (it != data_.end())
		{
			return it - data_.begin();
		}
		return -1;
	}

	int getQuantity(const int dataRowIndex) const
	{
		auto it = find_if(data_.begin(), data_.end(), [&dataRowIndex](const Row& row) { return row.rowIndexInData == dataRowIndex; });
		if (it != data_.end())
		{
			return it->quantity.toInt();
		}
		return 0;
	}

	void setQuantity(const int dataRowIndex, const int newQuantity)
	{
		auto it = find_if(data_.begin(), data_.end(), [&dataRowIndex](const Row& row) { return row.rowIndexInData == dataRowIndex; });
		if (it != data_.end())
		{
			if (newQuantity > 0)
			{
				it->quantity = newQuantity;
			}
			else
			{
				data_.erase(it);
			}
		}
		else
		{
			if (newQuantity > 0)
			{
				Row row;
				row.rowIndexInData = dataRowIndex;
				row.quantity = newQuantity;
				row.used = 0; // TODO
				data_.push_back(row);
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

void Collection::load()
{
	pimpl_->load();
}

void Collection::save()
{
	pimpl_->save();
}

int Collection::getNumRows() const
{
	return pimpl_->getNumRows();
}

const QVariant& Collection::get(const int row, const ColumnType& column) const
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
