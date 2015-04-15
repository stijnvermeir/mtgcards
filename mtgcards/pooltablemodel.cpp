#include "pooltablemodel.h"
#include "manacost.h"

#include <QAbstractTableModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QSettings>

#include <vector>
#include <array>

using namespace std;

namespace {

layout_type_t to_layout_type_t(const QString& str)
{
	if (str == "split")
	{
		return layout_type_t::Split;
	}
	if (str == "flip")
	{
		return layout_type_t::Flip;
	}
	if (str == "double-faced")
	{
		return layout_type_t::DoubleFaced;
	}
	if (str == "token")
	{
		return layout_type_t::Token;
	}
	return layout_type_t::Normal;
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

class PoolTableModel::Pimpl : public virtual QAbstractTableModel
{
public:
	typedef array<QVariant, mtg::TotalColumnCount> Row;
	vector<Row> data_;

	Pimpl()
		: data_()
	{
		loadData();
	}

	void loadData()
	{
		beginResetModel();
		data_.clear();

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

			data_.reserve(numCards);

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
					r[mtg::Set] = setName;
					r[mtg::SetCode] = setCode;
					r[mtg::SetReleaseDate] = setReleaseDate;
					r[mtg::SetType] = setType;
					r[mtg::Block] = block;

					// card
					r[mtg::Name] = card["name"].toString();
					r[mtg::Names] = jsonArrayToStringList(card["names"].toArray());
					r[mtg::ManaCost] = QVariant::fromValue(ManaCost(card["manaCost"].toString()));
					r[mtg::CMC] = card["cmc"].toInt();
					r[mtg::Color] = jsonArrayToStringList(card["colors"].toArray());
					r[mtg::Type] = card["type"].toString();
					r[mtg::SuperTypes] = jsonArrayToStringList(card["supertypes"].toArray());
					r[mtg::Types] = jsonArrayToStringList(card["types"].toArray());
					r[mtg::SubTypes] = jsonArrayToStringList(card["subtypes"].toArray());
					r[mtg::Rarity] = card["rarity"].toString();
					r[mtg::Text] = card["text"].toString();
					r[mtg::Flavor] = card["flavor"].toString();
					r[mtg::Artist] = card["artist"].toString();
					r[mtg::Power] = card["power"].toString();
					r[mtg::Toughness] = card["toughness"].toString();
					r[mtg::Loyalty] = card["loyalty"].toInt();

					// hidden
					r[mtg::Layout] = card["layout"].toString();
					r[mtg::ImageName] = card["imageName"].toString();

					data_.push_back(r);
				}
			}
		}
		endResetModel();
	}

	virtual int rowCount(const QModelIndex& ) const
	{
		return data_.size();
	}

	virtual int columnCount(const QModelIndex& ) const
	{
		return mtg::VisibleColumnCount;
	}

	virtual QVariant data(const QModelIndex& index, int role) const
	{
		if (index.isValid())
		{
			if (role == Qt::DisplayRole)
			{
				if (index.row() < static_cast<int>(data_.size()) && index.column() < mtg::VisibleColumnCount)
				{
					const QVariant& ret = data_[index.row()][index.column()];
					if (ret.type() == QVariant::StringList)
					{
						return ret.toStringList().join("/");
					}
					return ret;
				}
			}
		}
		return QVariant();
	}

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal)
		{
			if (role == Qt::DisplayRole)
			{
				switch (section)
				{
					case mtg::Set:	return "Set";
					case mtg::SetCode: return "Set Code";
					case mtg::SetReleaseDate: return "Set Release Date";
					case mtg::SetType: return "Set Type";
					case mtg::Block: return "Block";
					case mtg::Name: return "Name";
					case mtg::Names: return "Names";
					case mtg::ManaCost: return "Mana Cost";
					case mtg::CMC: return "CMC";
					case mtg::Color: return "Color";
					case mtg::Type: return "Type";
					case mtg::SuperTypes: return "Super Types";
					case mtg::Types: return "Types";
					case mtg::SubTypes: return "Sub Types";
					case mtg::Rarity: return "Rarity";
					case mtg::Text: return "Text";
					case mtg::Flavor: return "Flavor";
					case mtg::Artist: return "Artist";
					case mtg::Power: return "Power";
					case mtg::Toughness: return "Toughness";
					case mtg::Loyalty: return "Loyalty";

					default: break;
				}
			}
		}
		return QVariant();
	}

	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		return QAbstractTableModel::flags(index);
	}
};

PoolTableModel::PoolTableModel()
	: pimpl_(new Pimpl())
{
	setSourceModel(pimpl_);
}

PoolTableModel::~PoolTableModel()
{
	delete pimpl_;
}

void PoolTableModel::reload()
{
	pimpl_->loadData();
}

std::pair<layout_type_t, QStringList> PoolTableModel::getPictureFilenames(int row)
{
	QStringList list;
	layout_type_t layout = layout_type_t::Normal;
	if (row < static_cast<int>(pimpl_->data_.size()))
	{
		const Pimpl::Row& card = pimpl_->data_[row];
		QSettings settings;
		QString prefix = settings.value("options/datasources/cardpicturedir").toString();
		QString notFoundImageFile = prefix + QDir::separator() + "Back.jpg";
		auto addToListLambda = [&list, &notFoundImageFile, &card](QString imageFile)
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
				QString imageName = card[mtg::ImageName].toString();
				imageName.replace(fileInfo.baseName().toLower(), "");
				QString suffix = "";
				if (!imageName.isEmpty())
				{
					suffix = tr(" [") + imageName + "]";
				}
				imageFile = fileInfo.path() + QDir::separator() + fileInfo.baseName() + suffix + ".jpg";
				if (QFileInfo::exists(imageFile))
				{
					list.push_back(imageFile);
				}
				else
				{
					imageFile.replace(".jpg", ".Full.jpg");
					if (QFileInfo::exists(imageFile))
					{
						list.push_back(imageFile);
					}
					else
					{
						qWarning() << imageFile << " not found! (" << card[mtg::ImageName].toString() << ")";
						list.push_back(notFoundImageFile);
					}
				}
			}
		};
		prefix += QDir::separator() + card[mtg::Set].toString() + QDir::separator();
		layout = to_layout_type_t(card[mtg::Layout].toString());
		if (layout == layout_type_t::Split || layout == layout_type_t::Flip)
		{
			QStringList names = card[mtg::Names].toStringList();
			QString imageFile = prefix + names.join("_") + ".jpg";
			addToListLambda(imageFile);
		}
		else
		if (layout == layout_type_t::DoubleFaced)
		{
			QStringList names = card[mtg::Names].toStringList();
			for (const auto& n : names)
			{
				QString imageFile = prefix + n + ".jpg";
				addToListLambda(imageFile);
			}
		}
		else
		if (layout == layout_type_t::Token)
		{
			prefix += tr("token") + QDir::separator();
			QString tokenName = card[mtg::ImageName].toString();
			tokenName[0] = tokenName[0].toUpper();
			QString imageFile = prefix + tokenName + ".jpg";
			addToListLambda(imageFile);
		}
		else
		{
			QString imageFile = prefix + card[mtg::Name].toString() + ".jpg";
			addToListLambda(imageFile);
		}
	}
	return make_pair(layout, list);
}
