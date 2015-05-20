#pragma once

#include "filter.h"
#include "filters/comparetype.h"

#include <QDateTime>
#include <QString>

class TimeFilterFunction : public FilterFunction
{
public:
	TimeFilterFunction();

	virtual const QString& getId() const;
	virtual bool apply(const QVariant& data) const;
	virtual QString getDescription() const;
	virtual QJsonObject toJson() const;
	virtual void fromJson(const QJsonObject& obj);
	virtual QWidget* createEditor(QWidget* parent) const;
	virtual void updateFromEditor(const QWidget* editor);
private:
	CompareType compareType_;
	QDateTime timestamp_;
};
