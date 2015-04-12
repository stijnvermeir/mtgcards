#pragma once

#include <QtCore>

enum class layout_type_t
{
	Normal,
	DoubleFaced,
	Flip,
	Split,
	Token
};

Q_DECLARE_METATYPE(layout_type_t)
