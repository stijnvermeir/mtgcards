#include "magiccarddata.h"

#include "manacost.h"
#include "settings.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include <array>
#include <vector>

using namespace std;
using namespace mtg;

namespace {

const size_t COLUMNS_SIZE = 23;
const array<ColumnType, COLUMNS_SIZE> COLUMNS =
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

QString removeAccents(QString s)
{
	static QString diacriticLetters;
	static QStringList noDiacriticLetters;
	if (diacriticLetters.isEmpty())
	{
		diacriticLetters = QString::fromUtf8("ŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ");
		noDiacriticLetters << "S"<<"OE"<<"Z"<<"s"<<"oe"<<"z"<<"Y"<<"Y"<<"u";
		noDiacriticLetters << "A"<<"A"<<"A"<<"A"<<"A"<<"A"<<"AE";
		noDiacriticLetters << "C"<<"E"<<"E"<<"E"<<"E"<<"I"<<"I"<<"I"<<"I";
		noDiacriticLetters << "D"<<"N"<<"O"<<"O"<<"O"<<"O"<<"O"<<"O";
		noDiacriticLetters << "U"<<"U"<<"U"<<"U"<<"Y"<<"s";
		noDiacriticLetters << "a"<<"a"<<"a"<<"a"<<"a"<<"a"<<"ae";
		noDiacriticLetters << "c"<<"e"<<"e"<<"e"<<"e"<<"i"<<"i"<<"i"<<"i";
		noDiacriticLetters << "o"<<"n"<<"o"<<"o"<<"o"<<"o"<<"o"<<"o";
		noDiacriticLetters << "u"<<"u"<<"u"<<"u"<<"y"<<"y";
	}

	QString output = "";
	for (int i = 0; i < s.length(); i++)
	{
		QChar c = s[i];
		int dIndex = diacriticLetters.indexOf(c);
		if (dIndex < 0)
		{
			output.append(c);
		}
		else
		{
			QString replacement = noDiacriticLetters[dIndex];
			output.append(replacement);
		}
	}
	return output;
}

} // namespace

struct CardData::Pimpl
{
	typedef array<QVariant, COLUMNS_SIZE> Row;
	vector<Row> data;

	Pimpl()
		: data()
	{
	}

	void reload()
	{
		data.clear();

		QFile file(Settings::instance().getPoolDataFile());
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
#if 0 // test card picture availability
	for (int i = 0; i < getNumRows(); ++i)
	{
		getPictureFilenames(i);
	}
#endif
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

std::pair<mtg::LayoutType, QStringList> CardData::getPictureFilenames(int row)
{
	QStringList list;
	mtg::LayoutType layout = mtg::LayoutType::Normal;
	if (row < getNumRows())
	{
		QString prefix = Settings::instance().getCardImageDir();
		QString notFoundImageFile = prefix + QDir::separator() + "Back.jpg";
		QString imageName = get(row, mtg::ColumnType::ImageName).toString();
		auto addToListLambda = [&list, &notFoundImageFile, &imageName](QString imageFile)
		{
			// replace special characters
			imageFile.replace("\xc2\xae", ""); // (R)
			imageFile.replace(":", "");
			imageFile.replace("?", "");
			imageFile.replace("\"", "");
			imageFile = removeAccents(imageFile);
			if (QFileInfo::exists(imageFile))
			{
				list.push_back(imageFile);
			}
			else
			{
				// try with a version tag
				QFileInfo fileInfo(imageFile);
				QString imageNameCopy = imageName;
				imageNameCopy.replace(fileInfo.baseName().toLower(), "");
				QString suffix = "";
				if (!imageNameCopy.isEmpty())
				{
					suffix = QString(" [") + imageNameCopy + "]";
				}
				imageFile = fileInfo.path() + QDir::separator() + fileInfo.baseName() + suffix + ".jpg";
				if (QFileInfo::exists(imageFile))
				{
					list.push_back(imageFile);
				}
				else
				{
					qWarning() << imageFile << " not found! (" << imageName << ")";
					list.push_back(notFoundImageFile);
				}
			}
		};
		prefix += QDir::separator() + get(row, mtg::ColumnType::Set).toString() + QDir::separator();
		layout = mtg::LayoutType(get(row, mtg::ColumnType::Layout).toString());
		if (layout == mtg::LayoutType::Split || layout == mtg::LayoutType::Flip)
		{
			QStringList names = get(row, mtg::ColumnType::Names).toStringList();
			QString imageFile = prefix + names.join("_") + ".jpg";
			addToListLambda(imageFile);
		}
		else
		if (layout == mtg::LayoutType::DoubleFaced)
		{
			QStringList names = get(row, mtg::ColumnType::Names).toStringList();
			for (const auto& n : names)
			{
				QString imageFile = prefix + n + ".jpg";
				addToListLambda(imageFile);
			}
		}
		else
		if (layout == mtg::LayoutType::Token)
		{
			prefix += QString("token") + QDir::separator();
			QString tokenName = get(row, mtg::ColumnType::ImageName).toString();
			tokenName[0] = tokenName[0].toUpper();
			QString imageFile = prefix + tokenName + ".jpg";
			addToListLambda(imageFile);
		}
		else
		{
			QString imageFile = prefix + get(row, mtg::ColumnType::Name).toString() + ".jpg";
			addToListLambda(imageFile);
		}
	}
	return make_pair(layout, list);
}
