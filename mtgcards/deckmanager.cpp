#include "deckmanager.h"

#include"settings.h"

#include <QVector>
#include <QDir>
#include <QDebug>

struct DeckManager::Pimpl
{
	DeckManager* self_;
	QVector<QSharedPointer<Deck>> decks_;

	Pimpl(DeckManager* self)
		: self_(self)
	{
		QDir deckDir(Settings::instance().getDecksDir());
		QStringList nameFilters;
		nameFilters << "*.deck";
		QFileInfoList entries = deckDir.entryInfoList(nameFilters, QDir::Files);
		for (const QFileInfo& entry : entries)
		{
			addDeck(entry.absoluteFilePath());
		}
	}

	QSharedPointer<Deck> addDeck(const QString& id)
	{
		// qDebug() << "Adding " << id;
		QSharedPointer<Deck> deck;
		if (id.isEmpty())
		{
			deck = QSharedPointer<Deck>::create();
		}
		else
		{
			deck = QSharedPointer<Deck>(new Deck(id));
		}
		connect(deck.data(), SIGNAL(changed()), self_, SIGNAL(deckChanged()));
		decks_.push_back(deck);
		return deck;
	}

	QSharedPointer<Deck> getDeck(const QString& id)
	{
		for (const QSharedPointer<Deck>& deck : decks_)
		{
			if (deck->getId() == id)
			{
				// qDebug() << "Found deck with id " << id;
				return deck;
			}
		}

		QSharedPointer<Deck> newDeck = addDeck(id);
		emit self_->deckChanged();
		return newDeck;
	}

	void closeDeck(const QSharedPointer<Deck>& deck)
	{
		if (deck && deck->hasUnsavedChanges())
		{
			if (deck->getFilename().isEmpty())
			{
				int index = decks_.indexOf(deck);
				decks_.removeAt(index);
				emit self_->deckChanged();
			}
			else
			{
				deck->reload();
			}
		}
	}

	int getUsedCount(const int dataRowIndex) const
	{
		int usedCount = 0;
		for (const QSharedPointer<Deck>& deck : decks_)
		{
			if (deck->isActive())
			{
				usedCount += deck->getQuantity(dataRowIndex);
			}
		}
		return usedCount;
	}

	QVector<QSharedPointer<Deck>> getDecksUsedIn(const int dataRowIndex) const
	{
		QVector<QSharedPointer<Deck>> decks;
		for (const QSharedPointer<Deck>& deck : decks_)
		{
			if (deck->isActive())
			{
				if (deck->getQuantity(dataRowIndex) > 0)
				{
					decks.push_back(deck);
				}
			}
		}
		return decks;
	}
};

DeckManager& DeckManager::instance()
{
	static DeckManager inst;
	return inst;
}

QSharedPointer<Deck> DeckManager::getDeck(const QString& id)
{
	return pimpl_->getDeck(id);
}

void DeckManager::closeDeck(const QSharedPointer<Deck>& deck)
{
	pimpl_->closeDeck(deck);
}

int DeckManager::getUsedCount(const int dataRowIndex) const
{
	return pimpl_->getUsedCount(dataRowIndex);
}

QVector<QSharedPointer<Deck>> DeckManager::getDecksUsedIn(const int dataRowIndex) const
{
	return pimpl_->getDecksUsedIn(dataRowIndex);
}

DeckManager::DeckManager()
	: pimpl_(new Pimpl(this))
{
}

DeckManager::~DeckManager()
{
}
