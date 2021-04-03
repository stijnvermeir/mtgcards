#pragma once

#include "filter.h"

class CurrentCommanderDeckFilterFunction : public FilterFunction
{
public:
	CurrentCommanderDeckFilterFunction();

	virtual const QString& getId() const;
	virtual bool apply(const QVariant& data) const;
	virtual QString getDescription() const;
	virtual QJsonObject toJson() const;
	virtual void fromJson(const QJsonObject& obj);
	virtual QWidget* createEditor(QWidget* parent) const;
	virtual void updateFromEditor(const QWidget* editor);
	virtual mtg::ColumnType getColumnOverride() const;
};
