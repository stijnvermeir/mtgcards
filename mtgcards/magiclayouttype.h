#pragma once

#include <QString>
#include <QMetaType>

namespace mtg {

class LayoutType
{
public:
	enum type_t
	{
		Normal,
		DoubleFaced,
		Flip,
		Split,
		Token,

		COUNT
	};

	LayoutType();
	LayoutType(const type_t value);
	LayoutType(const QString& stringValue);

	operator QString () const;
	operator type_t () const;

private:
	type_t value_;
};

} // namespace mtg

Q_DECLARE_METATYPE(mtg::LayoutType)
