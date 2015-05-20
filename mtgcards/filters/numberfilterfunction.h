#pragma once

#include "filter.h"
#include "filters/comparetype.h"

#include <QString>

class NumberFilterFunction : public FilterFunction
{
public:
	NumberFilterFunction();

	virtual const QString& getId() const;
	virtual bool apply(const QVariant& data) const;
	virtual QString getDescription() const;
	virtual QJsonObject toJson() const;
	virtual void fromJson(const QJsonObject& obj);
	virtual QWidget* createEditor(QWidget* parent) const;
	virtual void updateFromEditor(const QWidget* editor);
private:
	CompareType compareType_;
	double number_;
};
