#pragma once

#include "filter.h"

#include <QDate>
#include <QString>

class DateFilterFunction : public FilterFunction
{
public:
	class CompareType
	{
	public:
		enum type_t
		{
			GreaterThan,
			LessThan,

			COUNT
		};

		CompareType(const type_t value = GreaterThan);
		CompareType(const QString& stringValue);

		operator QString () const;
		operator type_t () const;

		static const std::vector<CompareType>& list();
	private:
		type_t value_;
	};

	DateFilterFunction();

	virtual const QString& getId() const;
	virtual bool apply(const QVariant& data) const;
	virtual QString getDescription() const;
	virtual QJsonObject toJson() const;
	virtual void fromJson(const QJsonObject& obj);
	virtual QWidget* createEditor(QWidget* parent) const;
	virtual void updateFromEditor(const QWidget* editor);
private:
	CompareType compareType_;
	QDate date_;
};
