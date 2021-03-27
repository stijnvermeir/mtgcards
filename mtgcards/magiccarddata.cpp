#include "magiccarddata.h"

#include "manacost.h"
#include "settings.h"
#include "onlinedatacache.h"
#include "util.h"
#include "tags.h"
#include "prices.h"

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
#include <QtSql>
#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace mtg;

namespace {

const QVector<ColumnType> COLUMNS =
{
	ColumnType::Id,
    ColumnType::SetName,
	ColumnType::SetCode,
	ColumnType::SetReleaseDate,
	ColumnType::SetType,
	ColumnType::Border,
	ColumnType::Name,
	ColumnType::Names,
	ColumnType::ManaCost,
	ColumnType::CMC,
	ColumnType::Color,
	ColumnType::Type,
	ColumnType::Rarity,
	ColumnType::Text,
	ColumnType::Power,
	ColumnType::Toughness,
	ColumnType::Loyalty,
	ColumnType::Layout,
	ColumnType::ImageName,
	ColumnType::IsLatestPrint,
    ColumnType::ColorIdentity,
    ColumnType::LegalityCommander,
	ColumnType::Uuid,
    ColumnType::ScryfallId,
    ColumnType::OtherFaceIds,
    ColumnType::Side,
    ColumnType::IsCompanion,
    ColumnType::CanBeCommander,
    ColumnType::IsAlternative,
    ColumnType::IsFullArt,
    ColumnType::IsExtendedArt,
    ColumnType::IsPromo,
    ColumnType::IsReprint
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

bool downloadPicture(const QString& scryfallId, const QString& filename, bool hq, bool backFace)
{
	 QNetworkAccessManager m;
	 QNetworkRequest request;
	 auto url = QString("https://api.scryfall.com/cards/%1?format=image&version=%2&face=%3").arg(scryfallId).arg(hq ? QString("large") : QString("border_crop")).arg(backFace ? QString("back") : QString("front"));
	 qDebug() << url;
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

double downloadPrice(const QString& scryfallId)
{
	QNetworkAccessManager m;
	QNetworkRequest request;
	auto url = QString("https://api.scryfall.com/cards/%1").arg(scryfallId);
	qDebug() << url;
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
	QJsonParseError parseError;
	auto json = QJsonDocument::fromJson(rawData, &parseError);
	if (parseError.error != QJsonParseError::NoError)
	{
		qDebug() << parseError.errorString();
		return 0;
	}
	return json["prices"]["eur"].toString().toDouble();
}

QSqlDatabase conn()
{
	return QSqlDatabase::database("allprintings");
}

QSqlError openDb()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "allprintings");
	db.setDatabaseName(Settings::instance().getPoolDataFile());
	if (!db.open()) return db.lastError();
	return QSqlError();
}

void closeDb()
{
	QSqlDatabase::removeDatabase("allprintings");
}

} // namespace

struct CardData::Pimpl
{
	typedef QVector<QVariant> Row;
	QVector<Row> data_;
	QVector<QVector<mtg::Ruling>> rulings_;
	QHash<QString, QHash<QString, int>> quickLookUpTable_;
    QHash<QString, QVector<int>> quickLookUpTableByNameOnly_;
	QHash<QString, int> quickLookUpTableByUuid_;

	Pimpl()
		: data_()
		, rulings_()
		, quickLookUpTable_()
        , quickLookUpTableByNameOnly_()
	    , quickLookUpTableByUuid_()
	{
		QSqlError err = openDb();
		if (err.isValid())
		{
			qDebug() << err;
		}
		err = load();
		if (err.isValid())
		{
			qDebug() << err;
		}
		closeDb();
	}

	QSqlError load()
	{
		data_.clear();
		rulings_.clear();
		quickLookUpTable_.clear();
		quickLookUpTableByNameOnly_.clear();
		quickLookUpTableByUuid_.clear();

		QHash<QString, QPair<int, QDate>> latestPrintHash;

		QSqlDatabase db = conn();

		QHash<QString, QVector<mtg::Ruling>> tempRulings;
		QSqlQuery queryRulings(db);
		if (!queryRulings.exec("SELECT uuid, date, text FROM rulings")) return queryRulings.lastError();
		tempRulings.reserve(queryRulings.size());
		while (queryRulings.next())
		{
			QString uuid = queryRulings.value("uuid").toString();
			mtg::Ruling ruling;
			ruling.date = queryRulings.value("date").toString();
			ruling.text = queryRulings.value("text").toString();
			tempRulings[uuid].push_back(ruling);
		}

		QHash<QString, QMap<QString, QString>> tempLegalities;
		QSqlQuery queryLegalities(db);
		if (!queryLegalities.exec("SELECT uuid, format, status FROM legalities")) return queryLegalities.lastError();
		tempLegalities.reserve(queryLegalities.size());
		while (queryLegalities.next())
		{
			QString uuid = queryLegalities.value("uuid").toString();
			QString format = queryLegalities.value("format").toString();
			QString status = queryLegalities.value("status").toString();
			tempLegalities[uuid].insert(format, status);
		}

		QSqlQuery q(db);
		QString qs;
		qs += "SELECT ";
		qs += "s.name, ";
		qs += "s.code, ";
		qs += "s.releaseDate, ";
		qs += "s.type, ";
		qs += "c.isOnlineOnly, ";
		qs += "c.availability, ";
		qs += "c.isAlternative, ";
		qs += "c.isFullArt, ";
		qs += "c.isPromo, ";
		qs += "c.isReprint, ";
		qs += "c.printings, ";
		qs += "c.keywords, ";
		qs += "c.leadershipSkills, ";
		qs += "c.frameEffects, ";
		qs += "c.convertedManaCost, ";
		qs += "c.borderColor, ";
		qs += "c.name, ";
		qs += "c.faceName, ";
		qs += "c.manaCost, ";
		qs += "c.colors, ";
		qs += "c.type, ";
		qs += "c.rarity, ";
		qs += "c.text, ";
		qs += "c.power, ";
		qs += "c.toughness, ";
		qs += "c.loyalty, ";
		qs += "c.colorIdentity, ";
		qs += "c.layout, ";
		qs += "c.uuid, ";
		qs += "c.otherFaceIds, ";
		qs += "c.side, ";
		qs += "c.variations, ";
		qs += "c.scryfallId ";
		qs += "FROM ";
		qs += "cards c ";
		qs += "JOIN ";
		qs += "sets s ";
		qs += "ON c.setCode = s.code ";
		qs += "ORDER BY ";
		qs += "s.code, c.name ";
		if (!q.exec(qs)) return q.lastError();
		int numCards = q.size();
		data_.reserve(numCards);
		rulings_.reserve(numCards);
		quickLookUpTable_.reserve(numCards);
		quickLookUpTableByUuid_.reserve(numCards);
		while (q.next())
		{
			bool onlineOnly = q.value("cards.isOnlineOnly").toBool();
			if (onlineOnly)
			{
				continue;
			}
			bool isPaper = q.value("cards.availability").toString().contains("paper");
			if (!isPaper)
			{
				continue;
			}
			QString setType = q.value("sets.type").toString();
			if (setType == "token" || setType == "memorabilia")
			{
				continue;
			}
			QString border = q.value("cards.borderColor").toString();
			QString cardType = q.value("cards.type").toString();
			bool unSet = false;
			if (setType == "funny" && (border == "silver" || (border == "borderless" && cardType.contains("Basic Land"))))
			{
				unSet = true;
			}
			if (!unSet && setType == "funny")
			{
				continue;
			}
			QString layout = q.value("cards.layout").toString();
			if (layout == "phenomenon" || layout == "planar" || layout == "scheme" || layout == "token" || layout == "vanguard")
			{
				continue;
			}
			if (cardType == "Conspiracy")
			{
				continue;
			}

			bool isAlternative = q.value("cards.isAlternative").toBool();
			bool isFullArt = q.value("cards.isFullArt").toBool();
			bool isExtendedArt = q.value("cards.frameEffects").toString().contains("extendedart");
			bool isPromo = q.value("cards.isPromo").toBool();
			bool isReprint = q.value("cards.isReprint").toBool();
			bool isOnlyPrint = !q.value("cards.printings").toString().contains(',') && q.value("cards.variations").isNull();
			bool includeInLatestPrintCheck = false;
			if (isOnlyPrint)
			{
				includeInLatestPrintCheck = true;
			}
			else if (!isAlternative && !isFullArt && !isExtendedArt && !isPromo
			         && setType != "box"
			         && setType != "from_the_vault"
			         && setType != "masterpiece"
			         && setType != "premium_deck"
			         && setType != "spellbook"
			         && setType != "starter")
			{
				includeInLatestPrintCheck = true;
			}

			QString setName = q.value("sets.name").toString();
			QString setCode = q.value("sets.code").toString().toUpper();
			QDate setReleaseDate = QDate::fromString(q.value("sets.releaseDate").toString(), "yyyy-MM-dd");

			Row r(COLUMNS.size());
			r[columnToIndex(ColumnType::Id)] = data_.size();
			// set
			r[columnToIndex(ColumnType::SetName)] = setName;
			r[columnToIndex(ColumnType::SetCode)] = setCode;
			r[columnToIndex(ColumnType::SetReleaseDate)] = setReleaseDate;
			r[columnToIndex(ColumnType::SetType)] = setType;

			// card
			double cmc = q.value("cards.convertedManaCost").toDouble();
			r[columnToIndex(ColumnType::Border)] = border;
			QString cardName = q.value("cards.name").toString();
			QStringList cardNames;
			cardNames.push_back(cardName);
			if (!q.value("cards.faceName").isNull())
			{
				cardNames = cardName.split(" // ");
				cardName = q.value("cards.faceName").toString();
			}
			r[columnToIndex(ColumnType::Name)] = cardName;
			r[columnToIndex(ColumnType::Names)] = cardNames;
			r[columnToIndex(ColumnType::ManaCost)] = QVariant::fromValue(ManaCost(q.value("cards.manaCost").toString(), cmc));
			r[columnToIndex(ColumnType::CMC)] = cmc;
			r[columnToIndex(ColumnType::Color)] = q.value("cards.colors").toString().split(",");
			r[columnToIndex(ColumnType::Type)] = cardType;
			r[columnToIndex(ColumnType::Rarity)] = q.value("cards.rarity").toString();
			r[columnToIndex(ColumnType::Text)] = q.value("cards.text").toString();
			r[columnToIndex(ColumnType::Power)] = q.value("cards.power").toString();
			r[columnToIndex(ColumnType::Toughness)] = q.value("cards.toughness").toString();
			r[columnToIndex(ColumnType::Loyalty)] = q.value("cards.loyalty").toString();
			auto colorIdentities = q.value("cards.colorIdentity").toString().split(",");
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

			bool isCompanion = q.value("cards.keywords").toString().contains("Companion");
			r[columnToIndex(ColumnType::IsCompanion)] = isCompanion;
			bool canBeCommander = q.value("cards.leadershipSkills").toString().contains("'commander': True");
			r[columnToIndex(ColumnType::CanBeCommander)] = canBeCommander;

			r[columnToIndex(ColumnType::IsAlternative)] = isAlternative;
			r[columnToIndex(ColumnType::IsPromo)] = isPromo;
			r[columnToIndex(ColumnType::IsFullArt)] = isFullArt;
			r[columnToIndex(ColumnType::IsExtendedArt)] = isExtendedArt;
			r[columnToIndex(ColumnType::IsReprint)] = isReprint;

			// misc
			r[columnToIndex(ColumnType::Layout)] = layout;

			QString uuid = q.value("cards.uuid").toString();
			r[columnToIndex(ColumnType::Uuid)] = uuid;

			r[columnToIndex(ColumnType::OtherFaceIds)] = q.value("cards.otherFaceIds").toString().split(",");
			r[columnToIndex(ColumnType::Side)] = q.value("cards.side").toString();

			// Generate image name
			auto imageName = removeAccents(cardName.toLower());
			if (!q.value("cards.variations").isNull())
			{
				QStringList variations = q.value("cards.variations").toString().split(",");
				variations.push_back(uuid);
				variations.sort();
				auto index = variations.indexOf(uuid);
				imageName = imageName + QString::number(index+1);
			}
			r[columnToIndex(ColumnType::ImageName)] = imageName;

			r[columnToIndex(ColumnType::ScryfallId)] = q.value("cards.scryfallId").toString();

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
			rulings_.push_back(tempRulings[uuid]);

			// Legalities
			const auto& legalities = tempLegalities[uuid];
			if (!legalities.empty())
			{
				if (legalities.contains("commander"))
				{
					r[columnToIndex(ColumnType::LegalityCommander)] = legalities["commander"];
				}
			}

			quickLookUpTable_[setCode + cardName][imageName] = data_.size();
			quickLookUpTableByNameOnly_[cardName.toLower()].push_back(data_.size());
			quickLookUpTableByUuid_[uuid] = data_.size();
			data_.push_back(r);
		}

		return QSqlError();
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
			if (column == ColumnType::Price)
			{
				return Prices::instance().getPrice(get(row, ColumnType::Uuid).toString());
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
        if (set.isEmpty())
        {
            const auto& rows = findRowsFast(name);
            for (const auto& row : rows)
            {
                if (get(row, ColumnType::IsLatestPrint).toBool())
                {
                    return row;
                }
            }
        }
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

	int findRowFastByUuid(const QString& uuid) const
	{
		return quickLookUpTableByUuid_.value(uuid, -1);
	}

    const QVector<int>& findRowsFast(const QString& name) const
    {
        auto it = quickLookUpTableByNameOnly_.find(name.toLower());
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
		QString scryfallId = data_[row][columnToIndex(ColumnType::ScryfallId)].toString();
		auto price = downloadPrice(scryfallId);
		if (price > 0)
		{
			QString set = data_[row][columnToIndex(ColumnType::SetCode)].toString();
			QString name = data_[row][columnToIndex(ColumnType::Name)].toString();
			OnlineDataCache& cache = OnlineDataCache::instance();
			cache.set(set, name, ColumnType::Price, price);
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

int CardData::findRowFastByUuid(const QString& uuid) const
{
	return pimpl_->findRowFastByUuid(uuid);
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
		QVariant scryfallId = get(row, mtg::ColumnType::ScryfallId);
		auto addToListLambda = [&picInfo, &notFoundImageFile, &hq, &doDownload](const QString& imageName, QString imageFile, const QVariant& scryfallId, bool backFace)
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
					if (doDownload && scryfallId.isValid() && downloadPicture(scryfallId.toString(), imageFile, hq, backFace))
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
		prefix += QDir::separator() + get(row, mtg::ColumnType::SetCode).toString() + QDir::separator();
		if (hq)
		{
			prefix += QString("hq") + QDir::separator();
		}
		picInfo.layout = mtg::LayoutType(get(row, mtg::ColumnType::Layout).toString());
		if (picInfo.layout == mtg::LayoutType::Split || picInfo.layout == mtg::LayoutType::Flip || picInfo.layout == mtg::LayoutType::Aftermath || picInfo.layout == mtg::LayoutType::Adventure)
		{
			auto side = get(row, mtg::ColumnType::Side).toString();
			if (side == "a")
			{
				QString imageFile = prefix + get(row, mtg::ColumnType::Name).toString().replace(":", "") + ".jpg";
				addToListLambda(imageName, imageFile, scryfallId, false);
			}
			else
			{
				auto otherFaceIds = get(row, mtg::ColumnType::OtherFaceIds).toStringList();
				for(const auto& otherFaceId : otherFaceIds)
				{
					int otherFaceRowIndex = findRowFastByUuid(otherFaceId);
					auto otherSide = get(otherFaceRowIndex, mtg::ColumnType::Side).toString();
					if (otherSide == "a")
					{
						QString otherImageName = get(otherFaceRowIndex, mtg::ColumnType::ImageName).toString();
						QString otherImageFile = prefix + get(otherFaceRowIndex, mtg::ColumnType::Name).toString().replace(":", "") + ".jpg";
						QVariant otherScryfallId = get(otherFaceRowIndex, mtg::ColumnType::ScryfallId);
						addToListLambda(otherImageName, otherImageFile, otherScryfallId, false);
						break;
					}
				}
			}
		}
		else
		if (picInfo.layout == mtg::LayoutType::Transform || picInfo.layout == mtg::LayoutType::ModalDFC)
		{
			QString imageFile = prefix + get(row, mtg::ColumnType::Name).toString().replace(":", "") + ".jpg";
			auto side = get(row, mtg::ColumnType::Side).toString();
			addToListLambda(imageName, imageFile, scryfallId, side == "b");

			auto otherFaceIds = get(row, mtg::ColumnType::OtherFaceIds).toStringList();
			for(const auto& otherFaceId : otherFaceIds)
			{
				int otherFaceRowIndex = findRowFastByUuid(otherFaceId);
				QString otherImageName = get(otherFaceRowIndex, mtg::ColumnType::ImageName).toString();
				QString otherImageFile = prefix + get(otherFaceRowIndex, mtg::ColumnType::Name).toString().replace(":", "") + ".jpg";
				QVariant otherScryfallId = get(otherFaceRowIndex, mtg::ColumnType::ScryfallId);
				auto otherSide = get(otherFaceRowIndex, mtg::ColumnType::Side).toString();
				addToListLambda(otherImageName, otherImageFile, otherScryfallId, otherSide == "b");
			}
		}
		else
		if (picInfo.layout == mtg::LayoutType::Token)
		{
			prefix += QString("token") + QDir::separator();
			QString tokenName = get(row, mtg::ColumnType::ImageName).toString();
			tokenName[0] = tokenName[0].toUpper();
			QString imageFile = prefix + tokenName.replace(":", "") + ".jpg";
			addToListLambda(imageName, imageFile, scryfallId, false);
		}
		else
		{
			QString imageFile = prefix + get(row, mtg::ColumnType::Name).toString().replace(":", "") + ".jpg";
			addToListLambda(imageName, imageFile, scryfallId, false);
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
