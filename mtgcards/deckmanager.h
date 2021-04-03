#pragma once

#include "deck.h"

#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QObject>
#include <QVector>

class DeckManager : public QObject
{
	Q_OBJECT

public:
	static DeckManager& instance();

	const QVector<QSharedPointer<Deck>>& getDecks() const;

	QSharedPointer<Deck> findDeck(const QString& id);
	QSharedPointer<Deck> getDeck(const QString& id);
	void closeDeck(const QSharedPointer<Deck>& deck);

	int getUsedCount(const int dataRowIndex) const;
    int getUsedAllCount(const int dataRowIndex) const;
	QVector<QSharedPointer<Deck>> getDecksUsedIn(const int dataRowIndex) const;

	QSharedPointer<Deck> getCurrentDeck();

signals:
	void deckChanged();

public slots:
	void currentDeckChanged(QString);

private:
	DeckManager();
	~DeckManager();
	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
