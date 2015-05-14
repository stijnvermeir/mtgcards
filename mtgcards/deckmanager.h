#pragma once

#include "deck.h"

#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QObject>

class DeckManager : public QObject
{
	Q_OBJECT

public:
	static DeckManager& instance();

	QSharedPointer<Deck> getDeck(const QString& id);
	void closeDeck(const QSharedPointer<Deck>& deck);

	int getUsedCount(const int dataRowIndex) const;

signals:
	void deckChanged();

private:
	DeckManager();
	~DeckManager();
	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};