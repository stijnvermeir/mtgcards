#include "magiclayouttype.h"

#include <array>
#include <algorithm>

using namespace std;
using namespace mtg;

namespace {

const array<QString, LayoutType::COUNT> NAMES =
{{
	"normal",
	"double-faced",
	"flipped",
	"split",
	"token"
}};

} // namespace

LayoutType::LayoutType()
	: value_(Normal)
{
}

LayoutType::LayoutType(const type_t value)
	: value_(value)
{
}

LayoutType::LayoutType(const QString& stringValue)
	: value_(Normal)
{
	auto it = find(NAMES.begin(), NAMES.end(), stringValue);
	if (it != NAMES.end())
	{
		value_ = static_cast<type_t>(it - NAMES.begin());
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
