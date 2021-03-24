#include "magiccolumntype.h"

#include "settings.h"

using namespace mtg;

namespace {

const QVector<QString> NAMES =
{
	"Id",
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
	"Owned",
	"IsLatestPrint",
	"MultiverseId",
	"PriceLowest",
	"PriceLowestFoil",
	"PriceAverage",
	"PriceTrend",
	"MkmProductId",
	"MkmMetaproductId",
	"Tags",
	"NotOwned",
    "ColorIdentity",
    "LegalityStandard",
    "LegalityModern",
    "LegalityLegacy",
    "LegalityVintage",
    "LegalityCommander",
    "QuantityAll",
    "OwnedAll",
    "UsedAll",
	"Uuid",
	"ScryfallId",
    "OtherFaceIds",
    "Side",
    "IsCompanion",
    "CanBeCommander",
    "IsAlternative",
    "IsFullArt",
    "IsExtendedArt",
    "IsPromo",
    "IsReprint"
};

const QVector<QString> DISPLAY_NAMES =
{
	"Id",
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
    "Own",
    "Latest print",
	"Multiverse Id",
	"Lowest Price (EX+)",
	"Lowest Price (Foil)",
	"Average Price",
	"Price Trend",
	"MKM Product Id",
	"MKM Metaproduct Id",
	"Tags",
    "OwnNot",
    "Color Identity",
    "Standard legal",
    "Modern legal",
    "Legacy legal",
    "Vintage legal",
    "Commander legal",
    "QtyAll",
    "OwnAll",
    "UsedAll",
	"Uuid",
	"ScryfallId",
    "OtherFaceIds",
    "Side",
    "Companion",
    "Commander",
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
