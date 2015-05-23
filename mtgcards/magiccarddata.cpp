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
#include <QHash>
#include <QDebug>

using namespace mtg;

namespace {

const QVector<ColumnType> COLUMNS =
{
	ColumnType::Set,
	ColumnType::SetCode,
	ColumnType::SetGathererCode,
	ColumnType::SetOldCode,
	ColumnType::SetReleaseDate,
	ColumnType::SetType,
	ColumnType::Block,
	ColumnType::OnlineOnly,
	ColumnType::Border,
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
};

QVector<int> generateColumnIndices()
{
	QVector<int> indices(ColumnType::COUNT, -1);
	for (int i = 0; i < COLUMNS.size(); ++i)
	{
		indices[COLUMNS[i]] = i;
	}
	return indices;
}

int columnToIndex(const ColumnType::type_t column)
{
	static const QVector<int> COLUMN_INDICES = generateColumnIndices();
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
	typedef QVector<QVariant> Row;
	QVector<Row> data_;
	QHash<QString, QHash<QString, int>> quickLookUpTable_;

	Pimpl()
		: data_()
		, quickLookUpTable_()
	{
		reload();
	}

	void reload()
	{
		data_.clear();
		quickLookUpTable_.clear();

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

			data_.reserve(numCards);
			quickLookUpTable_.reserve(numCards);

			for (const auto& s : obj)
			{
				auto set = s.toObject();
				auto setName = set["name"].toString();
				auto setCode = set["code"].toString();
				auto setGathererCode = setCode;
				if (set.contains("gathererCode"))
				{
					setGathererCode = set["gathererCode"].toString();
				}
				auto setOldCode = setCode;
				if (set.contains("oldCode"))
				{
					setOldCode = set["oldCode"].toString();
				}
				auto setReleaseDate = QDate::fromString(set["releaseDate"].toString(), "yyyy-MM-dd");
				auto setType = set["type"].toString();
				auto block = set["block"].toString();
				bool onlineOnly = false;
				if (set.contains("onlineOnly"))
				{
					onlineOnly = set["onlineOnly"].toBool();
				}
				auto border = set["border"].toString();
				for (const auto& c : set["cards"].toArray())
				{
					auto card = c.toObject();
					Row r(COLUMNS.size());
					// set
					r[columnToIndex(ColumnType::Set)] = setName;
					r[columnToIndex(ColumnType::SetCode)] = setCode;
					r[columnToIndex(ColumnType::SetGathererCode)] = setGathererCode;
					r[columnToIndex(ColumnType::SetOldCode)] = setOldCode;
					r[columnToIndex(ColumnType::SetReleaseDate)] = setReleaseDate;
					r[columnToIndex(ColumnType::SetType)] = setType;
					r[columnToIndex(ColumnType::Block)] = block;
					r[columnToIndex(ColumnType::OnlineOnly)] = onlineOnly;
					r[columnToIndex(ColumnType::Border)] = border;

					// card
					QString cardName = card["name"].toString();
					r[columnToIndex(ColumnType::Name)] = cardName;
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
					QString imageName = card["imageName"].toString();
					r[columnToIndex(ColumnType::ImageName)] = imageName;

					quickLookUpTable_[setCode + cardName][imageName] = data_.size();
					data_.push_back(r);
				}
			}
		}
	}

	int getNumRows() const
	{
		return data_.size();
	}

	const QVariant& get(const int row, const ColumnType& column) const
	{
		if (row >= 0 && row < getNumRows())
		{
			auto index = columnToIndex(column);
			if (index < COLUMNS.size())
			{
				return data_[row][index];
			}
		}
		static const QVariant EMPTY;
		return EMPTY;
	}

	int findRow(const QVector<QPair<ColumnType, QVariant>>& criteria) const
	{
		auto isMatch = [&criteria](const Row& row)
		{
			for (const auto& criterium : criteria)
			{
				auto columnIndex = columnToIndex(criterium.first);
				if (columnIndex < COLUMNS.size())
				{
					if (row[columnIndex] != criterium.second)
					{
						return false;
					}
				}
			}
			return true;
		};

		for (int rowIndex = 0; rowIndex < data_.size(); ++rowIndex)
		{
			if (isMatch(data_[rowIndex]))
			{
				return rowIndex;
			}
		}
		return -1;
	}

	int findRowFast(const QString& set, const QString& name, const QString& imageName) const
	{
		QString key = set + name;
		if (quickLookUpTable_.contains(key))
		{
			const QHash<QString, int> secondary = quickLookUpTable_[key];
			if (imageName.isEmpty())
			{
				return secondary.begin().value();
			}
			else
			{
				return secondary.value(imageName, -1);
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

int CardData::findRow(const QVector<QPair<ColumnType, QVariant>>& criteria) const
{
	return pimpl_->findRow(criteria);
}

int CardData::findRowFast(const QString& set, const QString& name, const QString& imageName) const
{
	return pimpl_->findRowFast(set, name, imageName);
}

QPair<mtg::LayoutType, QStringList> CardData::getPictureFilenames(int row)
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
					// qWarning() << imageFile << " not found! (" << imageName << ")";
					list.push_back(notFoundImageFile);
				}
			}
		};
		prefix += QDir::separator() + get(row, mtg::ColumnType::Set).toString().replace(":", "") + QDir::separator();
		layout = mtg::LayoutType(get(row, mtg::ColumnType::Layout).toString());
		if (layout == mtg::LayoutType::Split || layout == mtg::LayoutType::Flip)
		{
			QStringList names = get(row, mtg::ColumnType::Names).toStringList();
			QString imageFile = prefix + names.join("_").replace(":", "") + ".jpg";
			addToListLambda(imageFile);
		}
		else
		if (layout == mtg::LayoutType::DoubleFaced)
		{
			QStringList names = get(row, mtg::ColumnType::Names).toStringList();
			for (auto& n : names)
			{
				QString imageFile = prefix + n.replace(":", "") + ".jpg";
				addToListLambda(imageFile);
			}
		}
		else
		if (layout == mtg::LayoutType::Token)
		{
			prefix += QString("token") + QDir::separator();
			QString tokenName = get(row, mtg::ColumnType::ImageName).toString();
			tokenName[0] = tokenName[0].toUpper();
			QString imageFile = prefix + tokenName.replace(":", "") + ".jpg";
			addToListLambda(imageFile);
		}
		else
		{
			QString imageFile = prefix + get(row, mtg::ColumnType::Name).toString().replace(":", "") + ".jpg";
			addToListLambda(imageFile);
		}
	}
	return qMakePair(layout, list);
}
