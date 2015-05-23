#include "filters/comparetype.h"

namespace {

const QVector<QString> STRING_VALUES =
{
	"=",
	"≠",
	">",
	"≥",
	"<",
	"≤"
};

} // namespace

CompareType::CompareType(const type_t value)
	: value_(value)
{
}

CompareType::CompareType(const QString& stringValue)
	: value_(Equals)
{
	int index = STRING_VALUES.indexOf(stringValue);
	if (index != -1)
	{
		value_ = static_cast<type_t>(index);
	}
}

CompareType::operator QString () const
{
	return STRING_VALUES[value_];
}

CompareType::operator type_t () const
{
	return value_;
}

const QVector<CompareType>& CompareType::list()
{
	static QVector<CompareType> l;
	static bool ready = false;
	if (!ready)
	{
		l.reserve(COUNT);
		for (int i = 0; i < COUNT; ++i)
		{
			l.push_back(CompareType(static_cast<CompareType::type_t>(i)));
		}
		ready = true;
	}
	return l;
}
