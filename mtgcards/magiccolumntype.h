#pragma once

#include <QString>
#include <QVector>
#include <QMetaType>

namespace mtg {

class ColumnType
{
public:
	enum type_t
	{
		Id,
		Uuid,
		OtherFaceIds,
		ScryfallId,
		SetName,
		SetCode,
		SetReleaseDate,
		SetType,
		Name,
		Names,
		ManaCost,
		CMC,
		Color,
		ColorIdentity,
		Type,
		Rarity,
		Text,
		Power,
		Toughness,
		Loyalty,
		LegalityCommander,
		CanBeCommander,
		IsCompanion,
		Quantity,
		Used,
		Sideboard,
		Owned,
		NotOwned,
		QuantityAll,
		OwnedAll,
		UsedAll,
		Price,
		Tags,
		Border,
		Layout,
		Side,
		ImageName,
		IsLatestPrint,
		IsAlternative,
		IsFullArt,
		IsExtendedArt,
		IsPromo,
		IsReprint,

		COUNT,
		UNKNOWN = -1
	};

	ColumnType(const type_t value = UNKNOWN);
	ColumnType(const QString& stringValue);

	operator QString () const;
	operator type_t () const;
	ColumnType::type_t value() const;

	const QString& getDisplayName() const;

	static const QVector<ColumnType>& list();

	bool operator==(const ColumnType& other) const;
	bool operator==(ColumnType::type_t other) const;

private:
	type_t value_;
};

} // namespace mtg

Q_DECLARE_METATYPE(mtg::ColumnType)
