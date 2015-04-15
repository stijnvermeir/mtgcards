#pragma once

#include <QMetaType>
#include <QString>

class ManaCost
{
public:

	ManaCost()
		: text_()
	{}

	explicit ManaCost(const QString& text)
		: text_(text)
	{
	}

	const QString& getText() const
	{
		return text_;
	}

	QString getRichText() const;

private:
	QString text_;
};

Q_DECLARE_METATYPE(ManaCost)
