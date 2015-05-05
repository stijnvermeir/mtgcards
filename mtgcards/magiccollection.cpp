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
	Pimpl()
	{
		reload();
	}

	void reload()
	{

	}

	int getNumRows() const
	{
		return 0;
	}

	const QVariant& get(const int /*row*/, const ColumnType& /*column*/) const
	{
		static const QVariant EMPTY;
		return EMPTY;
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
