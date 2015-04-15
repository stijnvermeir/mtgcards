#pragma once

#include <QMetaType>

class RichText
{
public:

	RichText()
		: text_()
	{}

	explicit RichText(const QString& text)
		: text_(text)
	{
	}

	const QString& getText() const
	{
		return text_;
	}

private:
	QString text_;
};

Q_DECLARE_METATYPE(RichText)
