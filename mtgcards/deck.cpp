#include "deck.h"

#include "magiccarddata.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include <vector>

using namespace std;
using namespace mtg;

struct Deck::Pimpl
{
	struct Row
	{
		int rowIndexInData;
		QVariant quantity;
		QVariant sideboard;

		Row()
			: rowIndexInData(-1)
			, quantity(0)
			, sideboard(0) {}
	};
	vector<Row> data_;
	bool active_;
	QString filename_;
	QString id_;
	bool hasUnsavedChanges_;

	Pimpl()
		: data_()
		, active_(true)
		, filename_()
		, id_()
		, hasUnsavedChanges_(false)
	{
	}

	void reload()
	{
		if (!filename_.isEmpty())
		{
			load(filename_);
		}
	}

	void load(const QString& filename)
	{
		data_.clear();

		QFile file(filename);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QJsonDocument d = QJsonDocument::fromJson(QString(file.readAll()).toUtf8());
			QJsonObject obj = d.object();
			active_ = obj["active"].toBool();
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
				r.sideboard = card["Sideboard"].toInt();
				data_.push_back(r);
			}
		}

		filename_ = filename;
		hasUnsavedChanges_ = false;
	}

	void save(const QString& filename)
	{
		QJsonArray cards;
		for (const Row& r : data_)
		{
			QJsonObject cardObj;
			cardObj["Set"] = mtg::CardData::instance().get(r.rowIndexInData, ColumnType::SetCode).toString();
			cardObj["Name"] = mtg::CardData::instance().get(r.rowIndexInData, ColumnType::Name).toString();
			cardObj["Quantity"] = r.quantity.toInt();
			cardObj["Sideboard"] = r.sideboard.toInt();
			cards.append(cardObj);
		}
		QJsonObject obj;
		obj["cards"] = cards;
		obj["active"] = active_;
		QJsonDocument doc(obj);
		QFile file(filename);
		if (!file.open(QIODevice::WriteOnly))
		{
			qWarning() << "Failed to save to file " << file.fileName();
		}
		else
		{
			file.write(doc.toJson());
		}
		filename_ = filename;
		hasUnsavedChanges_ = false;
	}

	int getNumRows() const
	{
		return static_cast<int>(data_.size());
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

	const QVariant& get(const int row, const ColumnType& column) const
	{
		if (row >= 0 && row < getNumRows())
		{
			const Row& entry = data_[row];
			if (column == ColumnType::Quantity)
			{
				return entry.quantity;
			}
			if (column == ColumnType::Sideboard)
			{
				return entry.sideboard;
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
			if (newQuantity >= 0)
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
			if (newQuantity >= 0)
			{
				Row row;
				row.rowIndexInData = dataRowIndex;
				row.quantity = newQuantity;
				row.sideboard = 0;
				data_.push_back(row);
			}
		}
		hasUnsavedChanges_ = true;
	}

	int getSideboard(const int dataRowIndex) const
	{
		auto it = find_if(data_.begin(), data_.end(), [&dataRowIndex](const Row& row) { return row.rowIndexInData == dataRowIndex; });
		if (it != data_.end())
		{
			return it->sideboard.toInt();
		}
		return 0;
	}

	void setSideboard(const int dataRowIndex, const int newSideboard)
	{
		auto it = find_if(data_.begin(), data_.end(), [&dataRowIndex](const Row& row) { return row.rowIndexInData == dataRowIndex; });
		if (it != data_.end())
		{
			if (newSideboard >= 0)
			{
				it->sideboard = newSideboard;
			}
		}
		else
		{
			if (newSideboard >= 0)
			{
				Row row;
				row.rowIndexInData = dataRowIndex;
				row.quantity = 0;
				row.sideboard = newSideboard;
				data_.push_back(row);
			}
		}
		hasUnsavedChanges_ = true;
	}
};

Deck::Deck()
	: pimpl_(new Pimpl())
{
	static int counter = 1;
	pimpl_->id_ = QString("New deck ") + QString::number(counter);
	++counter;
}

Deck::Deck(const QString& file)
	: pimpl_(new Pimpl())
{
	load(file);
}

Deck::~Deck()
{
}

void Deck::reload()
{
	pimpl_->reload();
	emit changed();
}

void Deck::load(const QString& filename)
{
	pimpl_->load(filename);
	emit changed();
}

void Deck::save(const QString& filename)
{
	pimpl_->save(filename);
}

const QString& Deck::getFilename() const
{
	return pimpl_->filename_;
}

const QString& Deck::getId() const
{
	if (!pimpl_->filename_.isEmpty())
	{
		return pimpl_->filename_;
	}
	return pimpl_->id_;
}

QString Deck::getDisplayName() const
{
	if (pimpl_->filename_.isEmpty())
	{
		return pimpl_->id_;
	}
	else
	{
		return QFileInfo(pimpl_->filename_).baseName();
	}
}

bool Deck::hasUnsavedChanges() const
{
	return pimpl_->hasUnsavedChanges_;
}

int Deck::getNumRows() const
{
	return pimpl_->getNumRows();
}

int Deck::getNumCards() const
{
	return pimpl_->getNumCards();
}

const QVariant& Deck::get(const int row, const ColumnType& column) const
{
	return pimpl_->get(row, column);
}

int Deck::getDataRowIndex(const int row) const
{
	return pimpl_->getDataRowIndex(row);
}

int Deck::getRowIndex(const int dataRowIndex) const
{
	return pimpl_->getRowIndex(dataRowIndex);
}

int Deck::getQuantity(const int dataRowIndex) const
{
	return pimpl_->getQuantity(dataRowIndex);
}

void Deck::setQuantity(const int dataRowIndex, const int newQuantity)
{
	pimpl_->setQuantity(dataRowIndex, newQuantity);
	emit changed();
}

int Deck::getSideboard(const int dataRowIndex) const
{
	return pimpl_->getSideboard(dataRowIndex);
}

void Deck::setSideboard(const int dataRowIndex, const int newSideboard)
{
	pimpl_->setSideboard(dataRowIndex, newSideboard);
}

bool Deck::isActive() const
{
	return pimpl_->active_;
}

void Deck::setActive(bool active)
{
	pimpl_->active_ = active;
	pimpl_->hasUnsavedChanges_ = true;
	emit changed();
}
