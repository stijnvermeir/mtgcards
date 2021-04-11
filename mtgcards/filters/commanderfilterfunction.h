#pragma once

#include "filter.h"

#include <QRegularExpression>

class CommanderFilterFunction : public FilterFunction
{
public:
	CommanderFilterFunction();

	virtual const QString& getId() const;
	virtual bool apply(const QVariant& data) const;
	virtual QString getDescription() const;
	virtual QJsonObject toJson() const;
	virtual void fromJson(const QJsonObject& obj);
	virtual QWidget* createEditor(QWidget* parent) const;
	virtual void updateFromEditor(const QWidget* editor);
	virtual mtg::ColumnType getColumnOverride() const;
private:
	QRegularExpression regex_;
	QString input_;

	void updateRegex();
};
