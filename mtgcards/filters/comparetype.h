#pragma once

#include <QString>
#include <QVector>

class CompareType
{
public:
	enum type_t
	{
		Equals,
		NotEquals,
		GreaterThan,
		GreaterThanOrEquals,
		LessThan,
		LessThanOrEquals,

		COUNT
	};

	CompareType(const type_t value = Equals);
	CompareType(const QString& stringValue);

	operator QString () const;
	operator type_t () const;

	template <class T>
	bool compare(const T& lhs, const T& rhs) const
	{
		switch (value_)
		{
			default:
			case Equals:
				return lhs == rhs;
			case NotEquals:
				return lhs != rhs;
			case GreaterThan:
				return lhs > rhs;
			case GreaterThanOrEquals:
				return lhs >= rhs;
			case LessThan:
				return lhs < rhs;
			case LessThanOrEquals:
				return lhs <= rhs;
		}
	}

	static const QVector<CompareType>& list();
private:
	type_t value_;
};
