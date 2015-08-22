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

	explicit ManaCost(const QString& text, const double cmc)
		: text_(text)
		, cmc_(cmc)
	{
	}

	const QString& getText() const
	{
		return text_;
	}

	double getCmc() const
	{
		return cmc_;
	}

	QString getRichText() const;

	static QString replaceTagsWithSymbols(const QString& in, const int fontSizeInPt);

private:
	QString text_;
	double cmc_;
};

bool operator<(const ManaCost& lhs, const ManaCost& rhs);

Q_DECLARE_METATYPE(ManaCost)
