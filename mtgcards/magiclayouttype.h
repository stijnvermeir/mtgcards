#pragma once

#include <QString>

namespace mtg {

class LayoutType
{
public:
	enum type_t
	{
		Normal,
		Split,
		Aftermath,
		Flip,
		Transform,
		Meld,
		Leveler,
		Saga,
		Planar,
		Scheme,
		Vanguard,
		Token,
		DoubleFacedToken,
		Emblem,
		Augment,
		Host,

		COUNT
	};

	LayoutType(const type_t value = Normal);
	LayoutType(const QString& stringValue);

	operator QString () const;
	operator type_t () const;

private:
	type_t value_;
};

} // namespace mtg
