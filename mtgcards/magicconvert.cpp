#include "magicconvert.h"

#include "manacost.h"

namespace mtg {

QString toString(const QVariant& data)
{
	if (data.canConvert<ManaCost>())
	{
		ManaCost manaCost = qvariant_cast<ManaCost>(data);
		return manaCost.getText();
	}
	else
	if (data.canConvert<QStringList>())
	{
		return data.toStringList().join(" / ");
	}
	else
	{
		return data.toString();
	}
}

} // mtg
