#include "pooltablemodel.h"

#include <QAbstractTableModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QSettings>

#include <vector>
#include <array>

using namespace std;

class PoolTableModel::Pimpl : public virtual QAbstractTableModel
{
public:
	typedef array<QVariant, mtg::ColumnCount> Row;
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
			qDebug() << "Total num cards: " << numCards;


			data_.reserve(numCards);

			for (const auto& set : obj)
			{
				auto setName = set.toObject()["name"].toString();
				for (const auto& c : set.toObject()["cards"].toArray())
				{
					auto card = c.toObject();
					Row r;
					r[mtg::Set] = setName;
					r[mtg::Name] = card["name"].toString();
					if (!card["names"].toArray().empty())
					{
						QStringList names;
						for (const auto& n : card["names"].toArray())
						{
							names.push_back(n.toString());
						}
						r[mtg::Names] = names;
					}
					QStringList colors;
					for (const auto& c : card["colors"].toArray())
					{
						colors.push_back(c.toString());
					}
					r[mtg::Color] = colors;
					r[mtg::Layout] = card["layout"].toString();
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
		return Row().max_size();
	}

	virtual QVariant data(const QModelIndex& index, int role) const
	{
		if (index.isValid())
		{
			if (role == Qt::DisplayRole)
			{
				if (index.row() < static_cast<int>(data_.size()) && index.column() < mtg::ColumnCount)
				{
					const QVariant& ret = data_[index.row()][index.column()];
					if (ret.type() == QVariant::StringList)
					{
						return ret.toStringList().join(" / ");
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
					case mtg::Name: return "Name";
					case mtg::Names: return "Names";
					case mtg::Color: return "Color";
					case mtg::Layout: return "Layout";
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

QStringList PoolTableModel::getPictureFilenames(int row)
{
	qDebug() << "Getting picture filenames for row " << row;
	QStringList list;
	if (row < static_cast<int>(pimpl_->data_.size()))
	{
		QSettings settings;
		QString prefix = settings.value("options/datasources/cardpicturedir").toString();
		qDebug() << "prefix = " << prefix;
		QString notFoundImageFile = prefix + "/Back.jpg";
		qDebug() << "not found image file = " << notFoundImageFile;
		prefix += tr("/") + pimpl_->data_[row][mtg::Set].toString();
		qDebug() << "prefix = " << prefix;
		QString layout = pimpl_->data_[row][mtg::Layout].toString();
		if (layout == "split" || layout == "flip")
		{
			qDebug() << "split layout";
			QStringList names = pimpl_->data_[row][mtg::Names].toStringList();
			QString imageFile = prefix + "/" + names[0] + "_" + names[1] + ".jpg";
			qDebug() << imageFile;
			if (QFile(imageFile).exists())
			{
				qDebug() << "exists!";
				list.push_back(imageFile);
			}
			else
			{
				qDebug() << "doesn't exist :(";
				list.push_back(notFoundImageFile);
			}
		}
		else
		{
			QString imageFile = prefix + "/" + pimpl_->data_[row][mtg::Name].toString() + ".jpg";
			qDebug() << imageFile;
			if (QFile(imageFile).exists())
			{
				qDebug() << "exists!";
				list.push_back(imageFile);
			}
			else
			{
				qDebug() << "doesn't exist :(";
				list.push_back(notFoundImageFile);
			}
		}
	}
	return list;
}
