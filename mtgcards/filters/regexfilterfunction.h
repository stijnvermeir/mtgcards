#pragma once

#include "filter.h"

#include <QRegularExpression>

class RegexFilterFunction : public FilterFunction
{
public:
	RegexFilterFunction();

	const QRegularExpression& getRegex() const;
	void setRegex(const QRegularExpression& regex);

	virtual const QString& getId() const;
	virtual bool apply(const QVariant& data) const;
	virtual QString getDescription() const;
	virtual QJsonObject toJson() const;
	virtual void fromJson(const QJsonObject& obj);
	virtual QWidget* createEditor(QWidget* parent) const;
	virtual void updateFromEditor(const QWidget* editor);
private:
	QRegularExpression regex_;
};
