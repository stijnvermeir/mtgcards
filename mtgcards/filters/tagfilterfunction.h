#pragma once

#include "filter.h"

#include <QString>

class TagFilterFunction : public FilterFunction
{
public:
	TagFilterFunction();

	virtual const QString& getId() const;
	virtual bool apply(const QVariant& data) const;
	virtual QString getDescription() const;
	virtual QJsonObject toJson() const;
	virtual void fromJson(const QJsonObject& obj);
	virtual QWidget* createEditor(QWidget* parent) const;
	virtual void updateFromEditor(const QWidget* editor);
	virtual mtg::ColumnType getColumnOverride() const;
private:
	QString tag_;
};
