#pragma once

#include <QString>
#include <vector>

namespace mtg {

class ColumnType
{
public:
	enum type_t
	{
		Set,
		SetCode,
		SetReleaseDate,
		SetType,
		Block,
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

		COUNT,
		UNKNOWN = -1
	};

	ColumnType();
	ColumnType(const type_t value);
	ColumnType(const QString& stringValue);

	operator QString () const;
	operator type_t () const;

	static const std::vector<ColumnType>& list();

private:
	type_t value_;
};

} // namespace mtg
