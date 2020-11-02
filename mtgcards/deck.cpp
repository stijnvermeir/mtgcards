#include "deck.h"

#include "magiccarddata.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QDebug>

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
		    , sideboard(0){}
	};
	QVector<Row> data_;
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
			for (const auto c : cards)
			{
				QJsonObject card = c.toObject();
				auto set = card["Set"].toString();
				if (set == "NMS")
				{
					set = "NEM";
				}
				auto name = card["Name"].toString();
				auto imageName = card["ImageName"].toString();

				auto rowIndexInData = mtg::CardData::instance().findRowFast(set, name, imageName);
				if (rowIndexInData == -1)
				{
					rowIndexInData = mtg::CardData::instance().findRowFast(set, name);
					if (rowIndexInData == -1)
					{
						qDebug() << "Could not find" << set << name << "for deck " << filename << (active_ ? "(active)" : "");
					}
				}
				if (rowIndexInData != -1)
				{
					Row r;
					r.rowIndexInData = rowIndexInData;
					r.quantity = card["Quantity"].toInt();
					r.sideboard = card["Sideboard"].toInt();
					data_.push_back(r);
				}
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
			cardObj["ImageName"] = mtg::CardData::instance().get(r.rowIndexInData, ColumnType::ImageName).toString();
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
			if (column == ColumnType::Sideboard)
			{
				return entry.sideboard;
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
				row->quantity = newQuantity;
			}
			else
			{
				data_.erase(row);
			}
		}
		else
		{
			if (newQuantity >= 0)
			{
				Row newRow;
				newRow.rowIndexInData = dataRowIndex;
				newRow.quantity = newQuantity;
				newRow.sideboard = 0;
				data_.push_back(newRow);
			}
		}
		hasUnsavedChanges_ = true;
	}

	int getSideboard(const int dataRowIndex) const
	{
		auto row = findRow(dataRowIndex);
		if (row)
		{
			return row->sideboard.toInt();
		}
		return 0;
	}

	void setSideboard(const int dataRowIndex, const int newSideboard)
	{
		auto row = findRow(dataRowIndex);
		if (row)
		{
			if (newSideboard >= 0)
			{
				row->sideboard = newSideboard;
			}
		}
		else
		{
			if (newSideboard >= 0)
			{
				Row newRow;
				newRow.rowIndexInData = dataRowIndex;
				newRow.quantity = 0;
				newRow.sideboard = newSideboard;
				data_.push_back(newRow);
			}
		}
		hasUnsavedChanges_ = true;
	}

	QVector<QPair<int,int>> getQuantities() const
	{
		QVector<QPair<int,int>> quantities;

		for (const Row& row : data_)
		{
			if (row.quantity.toInt() > 0)
			{
				quantities.push_back(QPair<int, int>(row.rowIndexInData, row.quantity.toInt()));
			}
		}

		return quantities;
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

QVariant Deck::get(const int row, const ColumnType& column) const
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

QVector<QPair<int, int>> Deck::getQuantities() const
{
	return pimpl_->getQuantities();
}
