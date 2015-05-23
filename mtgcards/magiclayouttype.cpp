#include "magiclayouttype.h"

#include <QVector>

using namespace mtg;

namespace {

const QVector<QString> NAMES =
{
	"normal",
	"double-faced",
	"flip",
	"split",
	"token"
};

} // namespace

LayoutType::LayoutType(const type_t value)
	: value_(value)
{
}

LayoutType::LayoutType(const QString& stringValue)
	: value_(Normal)
{
	int index = NAMES.indexOf(stringValue);
	if (index != -1)
	{
		value_ = static_cast<type_t>(index);
	}
}

LayoutType::operator QString () const
{
	return NAMES[value_];
}

LayoutType::operator type_t () const
{
	return value_;
}
