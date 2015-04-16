#pragma once

#include <QMetaType>
#include <QString>

class ManaCost
{
public:

	ManaCost()
		: text_()
		, cmc_(0)
	{}

	explicit ManaCost(const QString& text, const int cmc)
		: text_(text)
		, cmc_(cmc)
	{
	}

	const QString& getText() const
	{
		return text_;
	}

	int getCmc() const
	{
		return cmc_;
	}

	QString getRichText() const;

private:
	QString text_;
	int cmc_;
};

bool operator<(const ManaCost& lhs, const ManaCost& rhs);

Q_DECLARE_METATYPE(ManaCost)
