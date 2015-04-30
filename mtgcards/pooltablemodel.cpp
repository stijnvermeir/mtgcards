#include "pooltablemodel.h"
#include "manacost.h"
#include "magiccarddata.h"

#include <QAbstractTableModel>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDebug>

#include <vector>
#include <array>

using namespace std;

namespace {

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

const vector<mtg::ColumnType> POOLTABLE_COLUMNS =
{
	mtg::ColumnType::Set,
	mtg::ColumnType::SetCode,
	mtg::ColumnType::SetReleaseDate,
	mtg::ColumnType::SetType,
	mtg::ColumnType::Block,
	mtg::ColumnType::Name,
	mtg::ColumnType::Names,
	mtg::ColumnType::ManaCost,
	mtg::ColumnType::CMC,
	mtg::ColumnType::Color,
	mtg::ColumnType::Type,
	mtg::ColumnType::SuperTypes,
	mtg::ColumnType::Types,
	mtg::ColumnType::SubTypes,
	mtg::ColumnType::Rarity,
	mtg::ColumnType::Text,
	mtg::ColumnType::Flavor,
	mtg::ColumnType::Artist,
	mtg::ColumnType::Power,
	mtg::ColumnType::Toughness,
	mtg::ColumnType::Loyalty
};

} // namespace

class PoolTableModel::Pimpl : public virtual QAbstractTableModel
{
public:

	Pimpl()
	{
		loadData();
	}

	void loadData()
	{
		beginResetModel();
		mtg::CardData::instance().reload();
		endResetModel();
	}

	virtual int rowCount(const QModelIndex& ) const
	{
		return mtg::CardData::instance().getNumRows();
	}

	virtual int columnCount(const QModelIndex& = QModelIndex()) const
	{
		return static_cast<int>(POOLTABLE_COLUMNS.size());
	}

	virtual QVariant data(const QModelIndex& index, int role) const
	{
		if (index.isValid())
		{
			if (role == Qt::DisplayRole)
			{
				if (index.row() < mtg::CardData::instance().getNumRows() && index.column() < columnCount())
				{
					const QVariant& ret = mtg::CardData::instance().get(index.row(), POOLTABLE_COLUMNS[index.column()]);
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
				if (section >= 0 && section < columnCount())
				{
					return static_cast<QString>(POOLTABLE_COLUMNS[section]);
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
	setSourceModel(pimpl_.get());
}

PoolTableModel::~PoolTableModel()
{
}

void PoolTableModel::reload()
{
	pimpl_->loadData();
}

std::pair<mtg::LayoutType, QStringList> PoolTableModel::getPictureFilenames(int row)
{
	QStringList list;
	mtg::LayoutType layout = mtg::LayoutType::Normal;
	const auto& data = mtg::CardData::instance();
	if (row < data.getNumRows())
	{
		QSettings settings;
		QString prefix = settings.value("options/datasources/cardpicturedir").toString();
		QString notFoundImageFile = prefix + QDir::separator() + "Back.jpg";
		QString imageName = data.get(row, mtg::ColumnType::ImageName).toString();
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
					suffix = tr(" [") + imageNameCopy + "]";
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
		prefix += QDir::separator() + data.get(row, mtg::ColumnType::Set).toString() + QDir::separator();
		layout = mtg::LayoutType(data.get(row, mtg::ColumnType::Layout).toString());
		if (layout == mtg::LayoutType::Split || layout == mtg::LayoutType::Flip)
		{
			QStringList names = data.get(row, mtg::ColumnType::Names).toStringList();
			QString imageFile = prefix + names.join("_") + ".jpg";
			addToListLambda(imageFile);
		}
		else
		if (layout == mtg::LayoutType::DoubleFaced)
		{
			QStringList names = data.get(row, mtg::ColumnType::Names).toStringList();
			for (const auto& n : names)
			{
				QString imageFile = prefix + n + ".jpg";
				addToListLambda(imageFile);
			}
		}
		else
		if (layout == mtg::LayoutType::Token)
		{
			prefix += tr("token") + QDir::separator();
			QString tokenName = data.get(row, mtg::ColumnType::ImageName).toString();
			tokenName[0] = tokenName[0].toUpper();
			QString imageFile = prefix + tokenName + ".jpg";
			addToListLambda(imageFile);
		}
		else
		{
			QString imageFile = prefix + data.get(row, mtg::ColumnType::Name).toString() + ".jpg";
			addToListLambda(imageFile);
		}
	}
	return make_pair(layout, list);
}
