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
		UserDefined,

		COUNT,
		UNKNOWN = -1
	};

	ColumnType();
	ColumnType(const type_t value);
	ColumnType(const QString& stringValue);

	operator QString () const;
	operator type_t () const;

	const QString& getDisplayName() const;

	const UserColumn& userColumn() const;
	void setUserColumnIndex(int userColumnIndex);

	static const QVector<ColumnType>& list();

private:
	type_t value_;
	int userColumnIndex_;
};

} // namespace mtg

Q_DECLARE_METATYPE(mtg::ColumnType)
