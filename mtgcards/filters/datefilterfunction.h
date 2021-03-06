#pragma once

#include "filter.h"
#include "filters/comparetype.h"

#include <QDate>
#include <QString>

class DateFilterFunction : public FilterFunction
{
public:
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
