#include "filters/comparetype.h"

#include <array>
#include <algorithm>

using namespace std;

namespace {

const array<QString, CompareType::COUNT> STRING_VALUES =
{{
	"=",
	"≠",
	">",
	"≥",
	"<",
	"≤"
}};

} // namespace

CompareType::CompareType(const type_t value)
	: value_(value)
{
}

CompareType::CompareType(const QString& stringValue)
	: value_(Equals)
{
	auto it = find(STRING_VALUES.begin(), STRING_VALUES.end(), stringValue);
	if (it != STRING_VALUES.end())
	{
		value_ = static_cast<type_t>(it - STRING_VALUES.begin());
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
