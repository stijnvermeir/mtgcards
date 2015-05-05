#include "magiccollection.h"

#include "magiccarddata.h"
#include "settings.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include <array>
#include <vector>

using namespace std;
using namespace mtg;

struct Collection::Pimpl
{
	struct Row
	{
		int rowIndexInData;
		QVariant quantity;
		QVariant used;

		Row()
			: rowIndexInData(-1)
			, quantity(0)
			, used(0) {}
	};
	vector<Row> data_;

	Pimpl()
		: data_()
	{
		reload();
	}

	void reload()
	{

	}

	int getNumRows() const
	{
		return static_cast<int>(data_.size());
	}

	const QVariant& get(const int row, const ColumnType& column) const
	{
		if (row >= 0 && row < getNumRows())
		{
			const Row& entry = data_[row];
			if (column == ColumnType::Quantity)
			{
				return entry.quantity;
			}
			if (column == ColumnType::Used)
			{
				return entry.used;
			}
			return mtg::CardData::instance().get(entry.rowIndexInData, column);
		}
		static const QVariant EMPTY;
		return EMPTY;
	}

	int getQuantity(const int dataRowIndex) const
	{
		auto it = find_if(data_.begin(), data_.end(), [&dataRowIndex](const Row& row) { return row.rowIndexInData == dataRowIndex; });
		if (it != data_.end())
		{
			return it->quantity.toInt();
		}
		return 0;
	}

	void setQuantity(const int dataRowIndex, const int newQuantity)
	{
		auto it = find_if(data_.begin(), data_.end(), [&dataRowIndex](const Row& row) { return row.rowIndexInData == dataRowIndex; });
		if (it != data_.end())
		{
			it->quantity = newQuantity;
		}
		else
		{
			Row row;
			row.rowIndexInData = dataRowIndex;
			row.quantity = newQuantity;
			row.used = 0; // TODO
			data_.push_back(row);
		}
	}
};

Collection& Collection::instance()
{
	static Collection inst;
	return inst;
}

Collection::Collection()
	: pimpl_(new Pimpl())
{
}

Collection::~Collection()
{
}

void Collection::reload()
{
	pimpl_->reload();
}

int Collection::getNumRows() const
{
	return pimpl_->getNumRows();
}

const QVariant& Collection::get(const int row, const ColumnType& column) const
{
	return pimpl_->get(row, column);
}

int Collection::getQuantity(const int dataRowIndex) const
{
	return pimpl_->getQuantity(dataRowIndex);
}

void Collection::setQuantity(const int dataRowIndex, const int newQuantity)
{
	pimpl_->setQuantity(dataRowIndex, newQuantity);
}
