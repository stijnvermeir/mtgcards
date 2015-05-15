#include "magiccolumntype.h"
#include "usercolumn.h"
#include "settings.h"

#include <array>
#include <algorithm>

using namespace std;
using namespace mtg;

namespace {

const array<QString, ColumnType::COUNT> NAMES =
{{
	"Set",
	"SetCode",
	"SetGathererCode",
	"SetOldCode",
	"SetReleaseDate",
	"SetType",
	"Block",
	"OnlineOnly",
	"Border",
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
	"Sideboard",
	"UserDefined"
}};

const array<QString, ColumnType::COUNT> DISPLAY_NAMES =
{{
	"Set Name",
	"Set",
	"Set Gatherer Code",
	"Set Old Code",
	"Set Release Date",
	"Set Type",
	"Block",
	"Online Only",
	"Border",
	"Name",
	"Names",
	"Mana Cost",
	"CMC",
	"Color",
	"Type",
	"Super Types",
	"Types",
	"Sub Types",
	"Rarity",
	"Text",
	"Flavor",
	"Artist",
	"P",
	"T",
	"Loyalty",
	"Layout",
	"Image Name",
	"Qty",
	"Used",
	"SB",
	"User Defined"
}};

} // namespace

ColumnType::ColumnType()
	: value_(UNKNOWN)
	, userColumnIndex_(-1)
{
}

ColumnType::ColumnType(const type_t value)
	: value_(value)
	, userColumnIndex_(-1)
{
}

ColumnType::ColumnType(const QString& stringValue)
	: value_(UNKNOWN)
	, userColumnIndex_(-1)
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

const QString& ColumnType::getDisplayName() const
{
	if (value_ >= 0 && value_ < UserDefined)
	{
		return DISPLAY_NAMES[value_];
	}
	if (value_ == UserDefined)
	{
		return userColumn().name_;
	}
	static const QString EMPTY;
	return EMPTY;
}

const UserColumn& ColumnType::userColumn() const
{
	if (value_ == UserDefined)
	{
		const auto& userColumns = Settings::instance().getUserColumns();
		if (userColumnIndex_ >= 0 && userColumnIndex_ < userColumns.size())
		{
			return userColumns[userColumnIndex_];
		}
	}
	static UserColumn EMPTY;
	return EMPTY;
}

void ColumnType::setUserColumnIndex(int userColumnIndex)
{
	userColumnIndex_ = userColumnIndex;
}

const QVector<ColumnType>& ColumnType::list()
{
	static QVector<ColumnType> l;
	static bool ready = false;
	if (!ready)
	{
		const auto& userColumns = Settings::instance().getUserColumns();
		l.reserve(UserDefined + userColumns.size());
		for (int i = 0; i < UserDefined; ++i)
		{
			l.push_back(ColumnType(static_cast<ColumnType::type_t>(i)));
		}
		for (int i = 0; i < userColumns.size(); ++i)
		{
			ColumnType columnType(UserDefined);
			columnType.userColumnIndex_ = i;
			l.push_back(columnType);
		}
		ready = true;
	}
	return l;
}
