#pragma once

#include <QString>
#include <QVector>
#include <QMetaType>

struct UserColumn;

namespace mtg {

class ColumnType
{
public:
	enum type_t
	{
		Id,
		Set,
		SetCode,
		SetGathererCode,
		SetOldCode,
		SetReleaseDate,
		SetType,
		Block,
		OnlineOnly,
		Border,
		Name,
		Names,
		ManaCost,
		CMC,
		Color,
		Type,
		SuperTypes,
		Types,
		SubTypes,
		Rarity,
		Text,
		Flavor,
		Artist,
		Power,
		Toughness,
		Loyalty,
		Layout,
		ImageName,
		Quantity,
		Used,
		Sideboard,
		Owned,
		IsLatestPrint,
		MultiverseId,
		PriceLowest,
		PriceLowestFoil,
		PriceAverage,
		PriceTrend,
		MkmProductId,
		MkmMetaproductId,
		Tags,
		NotOwned,
        ColorIdentity,
        LegalityStandard,
        LegalityModern,
        LegalityLegacy,
        LegalityVintage,
        LegalityCommander,
        QuantityAll,
        OwnedAll,
        UsedAll,
		UserDefined,

		COUNT,
		UNKNOWN = -1
	};

	ColumnType(const type_t value = UNKNOWN);
	ColumnType(const QString& stringValue);

	operator QString () const;
	operator type_t () const;
	ColumnType::type_t value() const;

	const QString& getDisplayName() const;

	const UserColumn& userColumn() const;
	void setUserColumnIndex(int userColumnIndex);
	int getUserColumnIndex() const;

	static const QVector<ColumnType>& list();

	bool operator==(const ColumnType& other) const;
	bool operator==(ColumnType::type_t other) const;

private:
	type_t value_;
	int userColumnIndex_;
};

} // namespace mtg

Q_DECLARE_METATYPE(mtg::ColumnType)
