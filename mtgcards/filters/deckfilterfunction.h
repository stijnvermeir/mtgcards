#pragma once

#include "filter.h"
#include "deck.h"

#include <QWeakPointer>

class DeckFilterFunction : public FilterFunction
{
public:
	DeckFilterFunction();

	virtual const QString& getId() const;
	virtual bool apply(const QVariant& data) const;
	virtual QString getDescription() const;
	virtual QJsonObject toJson() const;
	virtual void fromJson(const QJsonObject& obj);
	virtual QWidget* createEditor(QWidget* parent) const;
	virtual void updateFromEditor(const QWidget* editor);
private:
	QWeakPointer<Deck> deck_;
};
