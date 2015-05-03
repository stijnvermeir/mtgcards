#include "magiccolumntype.h"

#include <array>
#include <algorithm>

using namespace std;
using namespace mtg;

namespace {

const array<QString, ColumnType::COUNT> NAMES =
{{
	"Set",
	"SetCode",
	"SetReleaseDate",
	"SetType",
	"Block",
	"Name",
	"Names",
	"ManaCost",
	"CMC",
	"Color",
	"Type",
	"SuperTypes",
	"Types",
	"SubTypes",
	"Rarity",
	"Text",
	"Flavor",
	"Artist",
	"Power",
	"Toughness",
	"Loyalty",
	"Layout",
	"ImageName",
	"Quantity",
	"Used",
	"Sideboard"
}};

} // namespace

ColumnType::ColumnType()
	: value_(UNKNOWN)
{
}

ColumnType::ColumnType(const type_t value)
	: value_(value)
{
}

ColumnType::ColumnType(const QString& stringValue)
	: value_(UNKNOWN)
{
	auto it = find(NAMES.begin(), NAMES.end(), stringValue);
	if (it != NAMES.end())
	{
		value_ = static_cast<type_t>(it - NAMES.begin());
	}
}

ColumnType::operator QString () const
{
	if (value_ >= 0 && value_ < COUNT)
	{
		return NAMES[value_];
	}
	return "UNKNOWN";
}

ColumnType::operator type_t () const
{
	return value_;
}

const vector<ColumnType>& ColumnType::list()
{
	static vector<ColumnType> l;
	static bool ready = false;
	if (!ready)
	{
		l.reserve(COUNT);
		for (int i = 0; i < COUNT; ++i)
		{
			l.push_back(ColumnType(static_cast<ColumnType::type_t>(i)));
		}
		ready = true;
	}
	return l;
}
