#include "magiccarddata.h"

#include "manacost.h"

#include <QDate>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>

#include <array>
#include <vector>

using namespace std;
using namespace mtg;

namespace {

const array<ColumnType, 23> COLUMNS =
{{
	ColumnType::Set,
	ColumnType::SetCode,
	ColumnType::SetReleaseDate,
	ColumnType::SetType,
	ColumnType::Block,
	ColumnType::Name,
	ColumnType::Names,
	ColumnType::ManaCost,
	ColumnType::CMC,
	ColumnType::Color,
	ColumnType::Type,
	ColumnType::SuperTypes,
	ColumnType::Types,
	ColumnType::SubTypes,
	ColumnType::Rarity,
	ColumnType::Text,
	ColumnType::Flavor,
	ColumnType::Artist,
	ColumnType::Power,
	ColumnType::Toughness,
	ColumnType::Loyalty,
	ColumnType::Layout,
	ColumnType::ImageName
}};

array<size_t, ColumnType::COUNT> generateColumnIndices()
{
	array<size_t, ColumnType::COUNT> indices;
	indices.fill(COLUMNS.max_size());
	for (size_t i = 0; i < COLUMNS.size(); ++i)
	{
		indices[COLUMNS[i]] = i;
	}
	return indices;
}

size_t columnToIndex(const ColumnType column)
{
	static const array<size_t, ColumnType::COUNT> COLUMN_INDICES = generateColumnIndices();
	return COLUMN_INDICES[column];
}

QStringList jsonArrayToStringList(const QJsonArray& array)
{
	QStringList list;
	for (const auto& n : array)
	{
		list.push_back(n.toString());
	}
	return list;
}

} // namespace

struct CardData::Pimpl
{
	typedef array<QVariant, COLUMNS.max_size()> Row;
	vector<Row> data;

	Pimpl()
		: data()
	{
	}

	void reload()
	{
		data.clear();

		QSettings settings;
		QFile file(settings.value("options/datasources/allsetsjson").toString());
		if (file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QJsonDocument d = QJsonDocument::fromJson(QString(file.readAll()).toUtf8());
			QJsonObject obj = d.object();
			int numCards = 0;
			for (const auto& set : obj)
			{
				numCards += set.toObject()["cards"].toArray().size();
			}

			data.reserve(numCards);

			for (const auto& set : obj)
			{
				auto setName = set.toObject()["name"].toString();
				auto setCode = set.toObject()["code"].toString();
				auto setReleaseDate = QDate::fromString(set.toObject()["releaseDate"].toString(), "yyyy-MM-dd");
				auto setType = set.toObject()["type"].toString();
				auto block = set.toObject()["block"].toString();
				for (const auto& c : set.toObject()["cards"].toArray())
				{
					auto card = c.toObject();
					Row r;
					// set
					r[columnToIndex(ColumnType::Set)] = setName;
					r[columnToIndex(ColumnType::SetCode)] = setCode;
					r[columnToIndex(ColumnType::SetReleaseDate)] = setReleaseDate;
					r[columnToIndex(ColumnType::SetType)] = setType;
					r[columnToIndex(ColumnType::Block)] = block;

					// card
					r[columnToIndex(ColumnType::Name)] = card["name"].toString();
					r[columnToIndex(ColumnType::Names)] = jsonArrayToStringList(card["names"].toArray());
					r[columnToIndex(ColumnType::ManaCost)] = QVariant::fromValue(ManaCost(card["manaCost"].toString(), card["cmc"].toDouble()));
					r[columnToIndex(ColumnType::CMC)] = card["cmc"].toDouble();
					r[columnToIndex(ColumnType::Color)] = jsonArrayToStringList(card["colors"].toArray());
					r[columnToIndex(ColumnType::Type)] = card["type"].toString();
					r[columnToIndex(ColumnType::SuperTypes)] = jsonArrayToStringList(card["supertypes"].toArray());
					r[columnToIndex(ColumnType::Types)] = jsonArrayToStringList(card["types"].toArray());
					r[columnToIndex(ColumnType::SubTypes)] = jsonArrayToStringList(card["subtypes"].toArray());
					r[columnToIndex(ColumnType::Rarity)] = card["rarity"].toString();
					r[columnToIndex(ColumnType::Text)] = card["text"].toString();
					r[columnToIndex(ColumnType::Flavor)] = card["flavor"].toString();
					r[columnToIndex(ColumnType::Artist)] = card["artist"].toString();
					r[columnToIndex(ColumnType::Power)] = card["power"].toString();
					r[columnToIndex(ColumnType::Toughness)] = card["toughness"].toString();
					r[columnToIndex(ColumnType::Loyalty)] = card["loyalty"].toInt();

					// hidden
					r[columnToIndex(ColumnType::Layout)] = card["layout"].toString();
					r[columnToIndex(ColumnType::ImageName)] = card["imageName"].toString();

					data.push_back(r);
				}
			}
		}
	}

	int getNumRows() const
	{
		return static_cast<int>(data.size());
	}

	const QVariant& get(const int row, const ColumnType& column) const
	{
		if (row >= 0 && row < getNumRows())
		{
			auto index = columnToIndex(column);
			if (index < COLUMNS.max_size())
			{
				return data.at(row)[index];
			}
		}
		static const QVariant EMPTY;
		return EMPTY;
	}

	int findRow(const std::vector<std::pair<ColumnType, QVariant>>& criteria) const
	{
		auto isMatch = [&criteria](const Row& row)
		{
			for (const auto& criterium : criteria)
			{
				auto columnIndex = columnToIndex(criterium.first);
				if (columnIndex < COLUMNS.max_size())
				{
					if (row[columnIndex] != criterium.second)
					{
						return false;
					}
				}
			}
			return true;
		};

		for (size_t rowIndex = 0; rowIndex < data.size(); ++rowIndex)
		{
			if (isMatch(data[rowIndex]))
			{
				return rowIndex;
			}
		}
		return -1;
	}
};

CardData& CardData::instance()
{
	static CardData inst;
	return inst;
}

CardData::CardData()
	: pimpl_(new Pimpl())
{
}

CardData::~CardData()
{
}

void CardData::reload()
{
	pimpl_->reload();
}

int CardData::getNumRows() const
{
	return pimpl_->getNumRows();
}

const QVariant& CardData::get(const int row, const ColumnType& column) const
{
	return pimpl_->get(row, column);
}

int CardData::findRow(const vector<pair<ColumnType, QVariant>>& criteria) const
{
	return pimpl_->findRow(criteria);
}
