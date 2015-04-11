#include "pooltablemodel.h"

#include <QAbstractTableModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

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
		QFile file("/home/dev/tmp/AllSets.json");
		file.open(QIODevice::ReadOnly | QIODevice::Text);
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
			auto setName = set.toObject()["code"].toString();
			for (const auto& c : set.toObject()["cards"].toArray())
			{
				auto card = c.toObject();
				Row r;
				r[mtg::Set] = setName;
				if (!card["names"].toArray().empty())
				{
					QStringList names;
					for (const auto& n : card["names"].toArray())
					{
						names.push_back(n.toString());
					}
					r[mtg::Name] = names;
				}
				else
				{
					r[mtg::Name] = card["name"].toString();
				}
				QStringList colors;
				for (const auto& c : card["colors"].toArray())
				{
					colors.push_back(c.toString());
				}
				r[mtg::Color] = colors;
				data_.push_back(r);
			}
		}
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
				const QVariant& ret = data_[index.row()][index.column()];
				if (ret.type() == QVariant::StringList)
				{
					return ret.toStringList().join(" / ");
				}
				return ret;
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
					case 0:	return "Set";
					case 1: return "Name";
					case 2: return "Color";
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
