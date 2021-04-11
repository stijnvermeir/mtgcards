#include "deck.h"

#include "magiccarddata.h"
#include "categories.h"
#include "magicconvert.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QRegularExpression>
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
		QVariant manaValue;
		bool isCommander;

		Row()
			: rowIndexInData(-1)
			, quantity(0)
		    , sideboard(0)
		    , manaValue()
		    , isCommander(false)
		{}
	};
	QVector<Row> data_;
	QHash<QString, QStringList> categories_;
	bool active_;
	QString filename_;
	QString id_;
	bool hasUnsavedChanges_;
	QString colorIdentity_;
	QRegularExpression colorIdentityRegex_;

	Pimpl()
		: data_()
	    , categories_()
		, active_(true)
		, filename_()
		, id_()
		, hasUnsavedChanges_(false)
	    , colorIdentity_("WUBRG")
	    , colorIdentityRegex_()
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
					if (card.contains("ManaValue"))
					{
						r.manaValue = card["ManaValue"].toInt();
					}
					if (card.contains("Categories"))
					{
						auto arr = card["Categories"].toArray();
						QStringList& list = categories_[name];
						for (const auto& i : arr)
						{
							list.append(i.toString());
						}
					}
					if (card.contains("Commander"))
					{
						r.isCommander = card["Commander"].toBool();
					}
					data_.push_back(r);
				}
			}
			updateColorIdentity();
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
			auto cardName = mtg::CardData::instance().get(r.rowIndexInData, ColumnType::Name).toString();
			cardObj["Set"] = mtg::CardData::instance().get(r.rowIndexInData, ColumnType::SetCode).toString();
			cardObj["Name"] = cardName;
			cardObj["ImageName"] = mtg::CardData::instance().get(r.rowIndexInData, ColumnType::ImageName).toString();
			cardObj["Quantity"] = r.quantity.toInt();
			cardObj["Sideboard"] = r.sideboard.toInt();
			if (r.manaValue.isValid())
			{
				cardObj["ManaValue"] = r.manaValue.toInt();
			}
			if (categories_.contains(cardName) && !categories_[cardName].empty())
			{
				cardObj["Categories"] = QJsonArray::fromStringList(categories_[cardName]);
			}
			if (r.isCommander)
			{
				cardObj["Commander"] = true;
			}
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
			if (column == ColumnType::CMC && entry.manaValue.isValid())
			{
				return entry.manaValue;
			}
			if (column == ColumnType::Categories)
			{
				return getCategories(entry.rowIndexInData);
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

	void overrideManaValue(const int dataRowIndex, QVariant manaValue)
	{
		auto row = findRow(dataRowIndex);
		if (row)
		{
			auto newValue = manaValue.toInt();
			auto defaultValue = mtg::CardData::instance().get(row->rowIndexInData, mtg::ColumnType::CMC).toInt();
			if (row->manaValue.isValid())
			{
				auto currentValue = row->manaValue.toInt();
				if (newValue != currentValue)
				{
					if (newValue == -1 || newValue == defaultValue)
					{
						row->manaValue.clear();
					}
					else
					{
						row->manaValue = manaValue;
					}
					hasUnsavedChanges_ = true;
				}
			}
			else
			{
				if (newValue != -1 && newValue != defaultValue)
				{
					row->manaValue = manaValue;
					hasUnsavedChanges_ = true;
				}
			}
		}
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

	QStringList getCategories(const int dataRowIndex) const
	{
		auto row = findRow(dataRowIndex);
		if (row)
		{
			auto name = mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::Name).toString();
			return categories_.value(name);
		}
		return QStringList();
	}

	QStringList getCategoryCompletions(const int dataRowIndex) const
	{
		QStringList result;
		auto categories = Categories::instance().getCategories();
		auto row = findRow(dataRowIndex);
		if (row)
		{
			auto name = mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::Name).toString();
			if (categories_.contains(name))
			{
				auto cardCategories = categories_.value(name);
				for (const QString& category : categories)
				{
					if (cardCategories.contains(category))
					{
						result << ("- " + category);
					}
					else
					{
						result << ("+ " + category);
					}
				}
				return result;
			}
		}

		for (const QString& category : categories)
		{
			result << ("+ " + category);
		}
		return result;
	}

	void updateCategories(const int dataRowIndex, const QString& update)
	{
		auto row = findRow(dataRowIndex);
		if (!row)
		{
			return;
		}
		auto cardName = mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::Name).toString();
		auto categories = Categories::instance().getCategories();
		if (update.startsWith('+'))
		{
			auto catName = update.mid(1).trimmed();
			if (categories.contains(catName))
			{
				if (!categories_[cardName].contains(catName))
				{
					categories_[cardName].append(catName);
					hasUnsavedChanges_ = true;
				}
			}
		}
		else
		if (update.startsWith('-'))
		{
			auto catName = update.mid(1).trimmed();
			if (categories.contains(catName))
			{
				if (categories_.contains(cardName))
				{
					if (categories_[cardName].contains(catName))
					{
						categories_[cardName].removeAll(catName);
						hasUnsavedChanges_ = true;
					}
				}
			}
		}
	}

	bool isCommander(const int dataRowIndex) const
	{
		auto row = findRow(dataRowIndex);
		if (row)
		{
			return row->isCommander;
		}
		return false;
	}

	void setCommander(const int dataRowIndex, bool commander)
	{
		auto row = findRow(dataRowIndex);
		if (row)
		{
			if (row->isCommander != commander)
			{
				if (commander)
				{
					bool hasPartner = false;
					bool hasPartnerWith = false;
					QString mustContain;
					if (mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::Text).toString().contains("Partner"))
					{
						hasPartner = true;
						hasPartnerWith = mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::Text).toString().contains("Partner with");
						if (hasPartnerWith)
						{
							mustContain = "Partner with " + mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::Name).toString();
						}
					}
					bool foundPartner = false;
					for (Row& r : data_)
					{
						if (r.isCommander)
						{
							if (hasPartnerWith)
							{
								bool isPartner = mtg::CardData::instance().get(r.rowIndexInData, mtg::ColumnType::Text).toString().contains(mustContain);
								if (!isPartner)
								{
									r.isCommander = false;
								}
							}
							else
							if (hasPartner)
							{
								bool otherHasPartner = mtg::CardData::instance().get(r.rowIndexInData, mtg::ColumnType::Text).toString().contains("Partner");
								if (otherHasPartner)
								{
									if (mtg::CardData::instance().get(r.rowIndexInData, mtg::ColumnType::Text).toString().contains("Partner with"))
									{
										r.isCommander = false;
									}
									else if (!foundPartner)
									{
										foundPartner = true;
									}
									else
									{
										r.isCommander = false;
									}
								}
								else
								{
									r.isCommander = false;
								}
							}
							else
							{
								r.isCommander = false;
							}
						}
					}
				}
				row->isCommander = commander;
				hasUnsavedChanges_ = true;
				updateColorIdentity();
			}
		}
	}

	void updateColorIdentity()
	{
		QString wubrg = "WUBRG";
		QMap<QChar,bool> colors;
		for (QChar c : wubrg)
		{
			colors[c] = false;
		}
		bool hasCommander = false;
		for (Row& r : data_)
		{
			if (r.isCommander)
			{
				auto colId = mtg::toString(mtg::CardData::instance().get(r.rowIndexInData, mtg::ColumnType::ColorIdentity));
				for (QChar c : wubrg)
				{
					colors[c] = colors[c] || colId.contains(c);
				}
				hasCommander = true;
			}
		}
		if (hasCommander)
		{
			colorIdentity_.clear();
			for (QChar c : wubrg)
			{
				if (colors[c])
				{
					colorIdentity_.append(c);
				}
			}
		}
		else
		{
			colorIdentity_ = wubrg;
		}
		updateColorIdentityRegex();
	}

	QString getColorIdentity() const
	{
		return colorIdentity_;
	}

	void updateColorIdentityRegex()
	{
		QString pattern = "[";
		for (QChar c : "WUBRG")
		{
			if (!colorIdentity_.contains(c))
			{
				pattern.append(c);
			}
		}
		pattern.append(']');
		colorIdentityRegex_.setPattern(pattern);
	}

	bool matchesColorIdentity(const QString& colorId)
	{
		return !colorIdentityRegex_.match(colorId).hasMatch();
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

void Deck::overrideManaValue(const int dataRowIndex, QVariant manaValue)
{
	pimpl_->overrideManaValue(dataRowIndex, manaValue);
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

QStringList Deck::getCategories(const int dataRowIndex) const
{
	return pimpl_->getCategories(dataRowIndex);
}

QStringList Deck::getCategoryCompletions(const int dataRowIndex) const
{
	return pimpl_->getCategoryCompletions(dataRowIndex);
}

void Deck::updateCategories(const int dataRowIndex, const QString& update)
{
	pimpl_->updateCategories(dataRowIndex, update);
}

bool Deck::isCommander(const int dataRowIndex) const
{
	return pimpl_->isCommander(dataRowIndex);
}

void Deck::setCommander(const int dataRowIndex, bool commander)
{
	pimpl_->setCommander(dataRowIndex, commander);
}

QString Deck::getColorIdentity() const
{
	return pimpl_->getColorIdentity();
}

bool Deck::matchesColorIdentity(const QString& colorId)
{
	return pimpl_->matchesColorIdentity(colorId);
}
