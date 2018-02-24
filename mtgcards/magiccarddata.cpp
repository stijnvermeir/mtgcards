#include "magiccarddata.h"

#include "manacost.h"
#include "settings.h"
#include "onlinedatacache.h"
#include "util.h"
#include "tags.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHash>
#include <QDebug>
#include <QThread>

using namespace mtg;

namespace {

const QVector<ColumnType> COLUMNS =
{
	ColumnType::Id,
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
	ColumnType::ImageName,
	ColumnType::IsLatestPrint,
    ColumnType::MultiverseId,
    ColumnType::ColorIdentity,
    mtg::ColumnType::LegalityStandard,
    mtg::ColumnType::LegalityModern,
    mtg::ColumnType::LegalityLegacy,
    mtg::ColumnType::LegalityVintage,
    mtg::ColumnType::LegalityCommander
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

bool downloadPicture(int multiverseId, const QString& filename, bool hq)
{
	 QNetworkAccessManager m;
	 QNetworkRequest request;
	 auto url = QString("https://api.scryfall.com/cards/multiverse/%1?format=image&version=%2").arg(multiverseId).arg((hq ? QString("large") : QString("border_crop")));
	 request.setUrl(url);
	 request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	 QScopedPointer<QNetworkReply> reply(m.get(request));
	 QEventLoop loop;
	 QObject::connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
	 loop.exec();
	 QThread::msleep(100); // Scryfall API asks to wait a bit between requests.
	 if (reply->error())
	 {
		 qDebug() << reply->error() << reply->errorString();
		 return false;
	 }
	 auto rawData = reply->readAll();
	 QFileInfo fi(filename);
	 fi.absoluteDir().mkpath(fi.absolutePath());
	 QImage picture = QImage::fromData(rawData);
	 return picture.save(filename);
}

} // namespace

struct CardData::Pimpl
{
	typedef QVector<QVariant> Row;
	QVector<Row> data_;
	QVector<QVector<mtg::Ruling>> rulings_;
	QHash<QString, QHash<QString, int>> quickLookUpTable_;
    QHash<QString, QVector<int>> quickLookUpTableByNameOnly_;

	Pimpl()
		: data_()
		, rulings_()
		, quickLookUpTable_()
        , quickLookUpTableByNameOnly_()
	{
		load();
	}

	void load()
	{
		data_.clear();
		quickLookUpTable_.clear();

		QFile file(Settings::instance().getPoolDataFile());
		if (file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QJsonDocument d = QJsonDocument::fromJson(file.readAll());
			QJsonObject obj = d.object();
			int numCards = 0;
			for (const auto& set : obj)
			{
				numCards += set.toObject()["cards"].toArray().size();
			}

			data_.reserve(numCards);
			rulings_.reserve(numCards);
			quickLookUpTable_.reserve(numCards);

			QHash<QString, QPair<int, QDate>> latestPrintHash;

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
				bool includeInLatestPrintCheck = (setType != "promo" && setType != "reprint" && !onlineOnly);
				for (const auto& c : set["cards"].toArray())
				{
					auto card = c.toObject();
					Row r(COLUMNS.size());
					r[columnToIndex(ColumnType::Id)] = data_.size();
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
                    auto colorIdentities = jsonArrayToStringList(card["colorIdentity"].toArray());
                    QString colorIdentityStr;
                    for (const auto& c : QString("WUBRG"))
                    {
                        if (colorIdentities.contains(c))
                        {
                            colorIdentityStr += QString{"{"} + c + "}";
                        }
                    }
                    if (colorIdentityStr.isEmpty())
                    {
                        colorIdentityStr = "{C}";
                    }
                    r[columnToIndex(ColumnType::ColorIdentity)] = QVariant::fromValue(ManaCost(colorIdentityStr, 0));

					// misc
					r[columnToIndex(ColumnType::Layout)] = card["layout"].toString();
					QString imageName = card["imageName"].toString();
					r[columnToIndex(ColumnType::ImageName)] = imageName;
					if (card.contains("multiverseid"))
					{
						r[columnToIndex(ColumnType::MultiverseId)] = card["multiverseid"].toInt();
					}
					r[columnToIndex(ColumnType::IsLatestPrint)] = false;

					if (includeInLatestPrintCheck)
					{
						if (latestPrintHash.contains(cardName))
						{
							auto& entry = latestPrintHash[cardName];
							if (setReleaseDate > entry.second)
							{
								// unflag the previous latest
								data_[entry.first][columnToIndex(ColumnType::IsLatestPrint)] = false;
								entry.first = data_.size();
								entry.second = setReleaseDate;
								r[columnToIndex(ColumnType::IsLatestPrint)] = true;
							}
						}
						else
						{
							latestPrintHash[cardName] = qMakePair(data_.size(), setReleaseDate);
							r[columnToIndex(ColumnType::IsLatestPrint)] = true;
						}
					}

					// rulings
					if (card.contains("rulings"))
					{
						QVector<mtg::Ruling> rulings;
						for (const QJsonValue& rv : card["rulings"].toArray())
						{
							QJsonObject r = rv.toObject();
							mtg::Ruling ruling;
							ruling.date = r["date"].toString();
							ruling.text = r["text"].toString();
							rulings.push_back(ruling);
						}
						rulings_.push_back(rulings);
					}
					else
					{
						rulings_.push_back(QVector<mtg::Ruling>());
					}

                    // Legalities
                    if (card.contains("legalities"))
                    {
                        for (const QJsonValue& lv : card["legalities"].toArray())
                        {
                            QJsonObject l = lv.toObject();
                            auto format = l["format"].toString();
                            auto legality = l["legality"].toString();
                            if (format == "Standard")
                            {
                                r[columnToIndex(ColumnType::LegalityStandard)] = legality;
                            }
                            else if (format == "Modern")
                            {
                                r[columnToIndex(ColumnType::LegalityModern)] = legality;
                            }
                            else if (format == "Legacy")
                            {
                                r[columnToIndex(ColumnType::LegalityLegacy)] = legality;
                            }
                            else if (format == "Vintage")
                            {
                                r[columnToIndex(ColumnType::LegalityVintage)] = legality;
                            }
                            else if (format == "Commander")
                            {
                                r[columnToIndex(ColumnType::LegalityCommander)] = legality;
                            }
                        }
                    }

					quickLookUpTable_[setCode + cardName][imageName] = data_.size();
                    quickLookUpTableByNameOnly_[cardName].push_back(data_.size());
					data_.push_back(r);
				}
			}
		}
	}

	int getNumRows() const
	{
		return data_.size();
	}

	QVariant get(const int row, const ColumnType& column) const
	{
		if (row >= 0 && row < getNumRows())
		{
			if (column == ColumnType::Tags)
			{
				QStringList names = data_[row][columnToIndex(ColumnType::Names)].toStringList();
				if (!names.empty())
				{
					return Tags::instance().getCardTags(names.join('/'));
				}
				return Tags::instance().getCardTags(data_[row][columnToIndex(ColumnType::Name)].toString());
			}
			else
			if (OnlineDataCache::isOnlineColumn(column))
			{
				return OnlineDataCache::instance().get(data_[row][columnToIndex(ColumnType::SetCode)].toString(), data_[row][columnToIndex(ColumnType::Name)].toString(), column);
			}
			else
			{
				auto index = columnToIndex(column);
				if (index < COLUMNS.size())
				{
					return data_[row][index];
				}
			}
		}
		return QVariant();
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

    const QVector<int>& findRowsFast(const QString& name) const
    {
        auto it = quickLookUpTableByNameOnly_.find(name);
        if (it != quickLookUpTableByNameOnly_.end())
        {
            return *it;
        }
        static const QVector<int> EMPTY;
        return EMPTY;
    }

    const QVector<int>& findReprintRows(const int row) const
    {
        return findRowsFast(get(row, ColumnType::Name).toString());
    }

	const QVector<mtg::Ruling>& getRulings(int row)
	{
		if (row >= 0 && row < getNumRows())
		{
			return rulings_[row];
		}
		static const QVector<mtg::Ruling> EMPTY;
		return EMPTY;
	}

	void fetchOnlineData(const int row)
	{
		QString set = data_[row][columnToIndex(ColumnType::SetCode)].toString();
		QString name = data_[row][columnToIndex(ColumnType::Name)].toString();
		mtg::LayoutType layout = mtg::LayoutType(data_[row][columnToIndex(ColumnType::Layout)].toString());
		QString search;
		if (layout == mtg::LayoutType::Split || layout == mtg::LayoutType::AfterMath || layout == mtg::LayoutType::DoubleFaced || layout == mtg::LayoutType::Flip)
		{
			QStringList names = data_[row][columnToIndex(ColumnType::Names)].toStringList();
			search = removeAccents(names.join(""));
		}
		else
		{
			search = removeAccents(name);
		}
		search.replace(" ", "");
		search.replace(",", "");
		search.replace("-", "");
		search.replace("'", "");
		search.replace("\"", "");
		QString expansion = data_[row][columnToIndex(ColumnType::Set)].toString();
		auto result = Util::mkmClient()->findProduct(search);
		for (const mkm::Product& p : result)
		{
			if (expansion.contains(p.expansion, Qt::CaseInsensitive))
			{
				OnlineDataCache& cache = OnlineDataCache::instance();
				cache.set(set, name, ColumnType::PriceLowest, p.priceGuide.lowExPlus);
				cache.set(set, name, ColumnType::PriceLowestFoil, p.priceGuide.lowFoil);
				cache.set(set, name, ColumnType::PriceAverage, p.priceGuide.avg);
				cache.set(set, name, ColumnType::PriceTrend, p.priceGuide.trend);
				cache.set(set, name, ColumnType::MkmProductId, p.idProduct);
				cache.set(set, name, ColumnType::MkmMetaproductId, p.idMetaproduct);
				return;
			}
		}
	}

	QStringList getCardTagCompletions(const int row)
	{
		if (row >= 0 && row < getNumRows())
		{
			QStringList names = data_[row][columnToIndex(ColumnType::Names)].toStringList();
			if (!names.empty())
			{
				return Tags::instance().getCardCompletions(names.join('/'));
			}
			return Tags::instance().getCardCompletions(data_[row][columnToIndex(ColumnType::Name)].toString());
		}
		return QStringList();
	}

	void updateTags(const int row, const QString& update)
	{
		if (row >= 0 && row < getNumRows())
		{
			QStringList names = data_[row][columnToIndex(ColumnType::Names)].toStringList();
			if (!names.empty())
			{
				return Tags::instance().updateCardTags(names.join('/'), update);
			}
			return Tags::instance().updateCardTags(data_[row][columnToIndex(ColumnType::Name)].toString(), update);
		}
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

int CardData::getNumRows() const
{
	return pimpl_->getNumRows();
}

QVariant CardData::get(const int row, const ColumnType& column) const
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

const QVector<int>& CardData::findRowsFast(const QString& name) const
{
    return pimpl_->findRowsFast(name);
}

const QVector<int>& CardData::findReprintRows(const int row) const
{
    return pimpl_->findReprintRows(row);
}

CardData::PictureInfo CardData::getPictureInfo(int row, bool hq, bool doDownload)
{
	PictureInfo picInfo;
	picInfo.layout = mtg::LayoutType::Normal;
	if (row < getNumRows())
	{
		QString prefix = Settings::instance().getCardImageDir();
		QString notFoundImageFile = prefix + QDir::separator() + "Back.jpg";
		QString imageName = get(row, mtg::ColumnType::ImageName).toString();
		QVariant multiverseId = get(row, mtg::ColumnType::MultiverseId);
		auto addToListLambda = [&picInfo, &notFoundImageFile, &imageName, &hq, &doDownload](QString imageFile, const QVariant& multiverseId)
		{
			// replace special characters
			imageFile.replace("\xc2\xae", ""); // (R)
			imageFile.replace("?", "");
			imageFile.replace("\"", "");
			imageFile = removeAccents(imageFile);
			if (QFileInfo::exists(imageFile))
			{
				picInfo.filenames << imageFile;
			}
			else
			{
				// try with a version tag
				QFileInfo fileInfo(imageFile);
				QString imageNameCopy = imageName;
				imageNameCopy.replace(fileInfo.completeBaseName().toLower(), "");
				QString suffix = "";
				if (!imageNameCopy.isEmpty())
				{
					suffix = QString(" [") + imageNameCopy + "]";
				}
				imageFile = fileInfo.path() + QDir::separator() + fileInfo.completeBaseName() + suffix + ".jpg";
				if (QFileInfo::exists(imageFile))
				{
					picInfo.filenames << imageFile;
				}
				else
				{
					// try downloading
					if (doDownload && multiverseId.isValid() && downloadPicture(multiverseId.toInt(), imageFile, hq))
					{
						picInfo.filenames << imageFile;
					}
					else
					{
						picInfo.filenames << notFoundImageFile;
						picInfo.missing << imageFile;
					}
				}
			}
		};
		prefix += QDir::separator() + get(row, mtg::ColumnType::Set).toString().replace(":", "") + QDir::separator();
		if (hq)
		{
			prefix += QString("hq") + QDir::separator();
		}
		picInfo.layout = mtg::LayoutType(get(row, mtg::ColumnType::Layout).toString());
		if (picInfo.layout == mtg::LayoutType::Split || picInfo.layout == mtg::LayoutType::Flip)
		{
			QStringList names = get(row, mtg::ColumnType::Names).toStringList();
			QString imageFile = prefix + names.join("_").replace(":", "") + ".jpg";
			addToListLambda(imageFile, multiverseId);
		}
		else
		if (picInfo.layout == mtg::LayoutType::DoubleFaced)
		{
			QStringList names = get(row, mtg::ColumnType::Names).toStringList();
			for (auto& n : names)
			{
				QString imageFile = prefix + n.replace(":", "") + ".jpg";
				int dataRowIndex = findRowFast(get(row, mtg::ColumnType::SetCode).toString(), n);
				addToListLambda(imageFile, get(dataRowIndex, mtg::ColumnType::MultiverseId));
			}
		}
		else
		if (picInfo.layout == mtg::LayoutType::Token)
		{
			prefix += QString("token") + QDir::separator();
			QString tokenName = get(row, mtg::ColumnType::ImageName).toString();
			tokenName[0] = tokenName[0].toUpper();
			QString imageFile = prefix + tokenName.replace(":", "") + ".jpg";
			addToListLambda(imageFile, multiverseId);
		}
		else
		{
			QString imageFile = prefix + get(row, mtg::ColumnType::Name).toString().replace(":", "") + ".jpg";
			addToListLambda(imageFile, multiverseId);
		}
	}
	return picInfo;
}

const QVector<mtg::Ruling>& CardData::getRulings(int row)
{
	return pimpl_->getRulings(row);
}

void CardData::fetchOnlineData(const int row)
{
	pimpl_->fetchOnlineData(row);
}

QStringList CardData::getCardTagCompletions(const int row)
{
	return pimpl_->getCardTagCompletions(row);
}

void CardData::updateTags(const int row, const QString& update)
{
	pimpl_->updateTags(row, update);
}
