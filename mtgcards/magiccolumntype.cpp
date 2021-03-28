#include "magiccolumntype.h"

#include "settings.h"

using namespace mtg;

namespace {

const QVector<QString> NAMES =
{
    "Id",
    "Uuid",
    "OtherFaceIds",
    "ScryfallId",
    "SetName",
    "SetCode",
    "SetReleaseDate",
    "SetType",
    "Name",
    "Names",
    "ManaCost",
    "CMC",
    "Color",
    "ColorIdentity",
    "Type",
    "Rarity",
    "Text",
    "Power",
    "Toughness",
    "Loyalty",
    "LegalityCommander",
    "CanBeCommander",
    "IsCompanion",
    "Quantity",
    "Used",
    "Sideboard",
    "Owned",
    "NotOwned",
    "QuantityAll",
    "OwnedAll",
    "UsedAll",
    "Price",
    "Tags",
    "Border",
    "Layout",
    "Side",
    "ImageName",
    "IsLatestPrint",
    "IsAlternative",
    "IsFullArt",
    "IsExtendedArt",
    "IsPromo",
    "IsReprint"
};

const QVector<QString> DISPLAY_NAMES =
{
	"Id",
    "Uuid",
    "OtherFaceIds",
    "ScryfallId",
	"Set Name",
	"Set",
    "Release",
	"Set Type",
	"Name",
	"Names",
	"Mana Cost",
	"CMC",
	"Color",
    "Color ID",
	"Type",
	"Rarity",
	"Text",
	"P",
	"T",
	"Loyalty",
    "Commander Legal",
    "Commander",
    "Companion",
    "Qty",
    "Used",
    "SB",
    "Own",
    "OwnNot",
    "QtyAll",
    "OwnAll",
    "UsedAll",
    "Price",
	"Tags",
    "Border",
    "Layout",
    "Side",
    "Imagename",
    "Latest print",
    "Alternative",
    "Full art",
    "Extended art",
    "Promo",
    "Reprint"
};

} // namespace

ColumnType::ColumnType(const type_t value)
	: value_(value)
{
}

ColumnType::ColumnType(const QString& stringValue)
	: value_(UNKNOWN)
{
	int index = NAMES.indexOf(stringValue);
	if (index != -1)
	{
		value_ = static_cast<type_t>(index);
	}
}

ColumnType::operator QString () const
{
	if (value_ >= 0 && value_ < NAMES.size())
	{
		return NAMES[value_];
	}
	return "UNKNOWN";
}

ColumnType::operator type_t () const
{
	return value_;
}

ColumnType::type_t ColumnType::value() const
{
	return value_;
}

const QString& ColumnType::getDisplayName() const
{
	if (value_ >= 0 && value_ < COUNT)
	{
		return DISPLAY_NAMES[value_];
	}
	static const QString EMPTY;
	return EMPTY;
}

const QVector<ColumnType>& ColumnType::list()
{
	static QVector<ColumnType> l;
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

bool ColumnType::operator==(const ColumnType& other) const
{
	return (value_ == other.value_);
}

bool ColumnType::operator==(ColumnType::type_t other) const
{
	return (value_ == other);
}
