#include "tags.h"
#include "settings.h"

#include <QtSql>
#include <QHash>
#include <QList>
#include <QDebug>

namespace {

QSqlDatabase conn()
{
	return QSqlDatabase::database("tags");
}

QSqlError initDb()
{
	const int DB_VERSION = 1;
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "tags");
	db.setDatabaseName(Settings::instance().getTagsDb());
	if (!db.open()) return db.lastError();

	if (!db.tables().contains("version")) // database does not exist yet
	{
		qDebug() << "Creating tags db ...";
		QSqlQuery q(db);

		// version table
		if (!q.exec("CREATE TABLE version(number integer primary key)")) return q.lastError();
		if (!q.prepare("INSERT INTO version(number) VALUES (?)")) return q.lastError();
		q.addBindValue(DB_VERSION);
		if (!q.exec()) return q.lastError();

		// tag table
		if (!q.exec("CREATE TABLE tag(id integer primary key, name text)")) return q.lastError();

		// card table
		if (!q.exec("CREATE TABLE card(id integer primary key, name text)")) return q.lastError();

		// cardtag table
		if (!q.exec("CREATE TABLE cardtag(id integer primary key, tag_id integer, card_id integer, "
					"FOREIGN KEY(tag_id) REFERENCES tag(id) ON UPDATE CASCADE ON DELETE CASCADE, "
					"FOREIGN KEY(card_id) REFERENCES card(id) ON UPDATE CASCADE ON DELETE CASCADE)")) return q.lastError();

		qDebug() << "Tags db created succesfully.";
	}
	else
	{
		QSqlQuery q(db);
		if (!q.exec("SELECT max(number) FROM version")) return q.lastError();
		if (!q.next()) return q.lastError();
		int currentVersion = q.value(0).toInt();
		if (currentVersion != DB_VERSION)
		{
			qDebug() << "Collection db version" << currentVersion;
			// for auto database model updates later
			qFatal("Invalid collection db version");
		}
	}

	return QSqlError();
}

} // namespace

struct Tags::Pimpl
{
	QHash<QString, int> tags_;
	QHash<int, QString> tagsReversed_;
	QHash<QString, int> cards_;
	QHash<int, QList<int>> cardTags_;

	Pimpl()
	{
		QSqlError err = initDb();
		if (err.isValid())
		{
			qDebug() << err;
		}
		err = load();
		if (err.isValid())
		{
			qDebug() << err;
		}
	}

	QSqlError load()
	{
		tags_.clear();
		cards_.clear();
		cardTags_.clear();

		QSqlDatabase db = conn();
		QSqlQuery q(db);
		if (!q.exec("SELECT id, name FROM tag")) return q.lastError();
		while (q.next())
		{
			auto id = q.value(0).toInt();
			auto name = q.value(1).toString();
			tags_[name] = id;
			tagsReversed_[id] = name;
		}
		if(!q.exec("SELECT id, name FROM card")) return q.lastError();
		while (q.next())
		{
			auto id = q.value(0).toInt();
			auto name = q.value(1).toString();
			cards_[name] = id;
		}
		if(!q.exec("SELECT tag_id, card_id FROM cardtag")) return q.lastError();
		while (q.next())
		{
			auto tagId = q.value(0).toInt();
			auto cardId = q.value(1).toInt();
			cardTags_[cardId].push_back(tagId);
		}
		return QSqlError();
	}

	QStringList getTags() const
	{
        auto tags = tags_.keys();
        tags.sort(Qt::CaseInsensitive);
        return tags;
	}

	void addTag(const QString& tag)
	{
		if (!tags_.contains(tag))
		{
			QSqlDatabase db = conn();
			QSqlQuery q(db);
			if (!q.prepare("INSERT INTO tag(name) VALUES(?)")) return;
			q.addBindValue(tag);
			if (!q.exec()) return;
			int tagId = q.lastInsertId().toInt();
			tags_[tag] = tagId;
			tagsReversed_[tagId] = tag;
		}
	}

	void removeTag(const QString& tag)
	{
		if (tags_.contains(tag))
		{
			int tagId = tags_[tag];
			QSqlDatabase db = conn();
			QSqlQuery q(db);
			if (!q.prepare("DELETE FROM tag WHERE id = ?")) return;
			q.addBindValue(tagId);
			if (!q.exec()) return;
			if (!q.prepare("DELETE FROM cardtag WHERE tag_id = ?")) return;
			q.addBindValue(tagId);
			if (!q.exec()) return;
			tags_.remove(tag);
			tagsReversed_.remove(tagId);
			for (QList<int>& tagList : cardTags_)
			{
				tagList.removeAll(tagId);
			}
		}
	}

	QStringList getCardTags(const QString& card) const
	{
		QStringList result;
		if (cards_.contains(card))
		{
			auto cardId = cards_[card];
			if (cardTags_.contains(cardId))
			{
				for (int tagId : cardTags_[cardId])
				{
					result << tagsReversed_[tagId];
				}
			}
		}
		return result;
	}

	QStringList getCardCompletions(const QString& card) const
	{
		QStringList result;
		if (cards_.contains(card))
		{
			auto cardId = cards_[card];
			if (cardTags_.contains(cardId))
			{
				const QList<int>& cardTags = cardTags_[cardId];
				for (int tagId : tagsReversed_.keys())
				{
					if (cardTags.contains(tagId))
					{
						result << ("- " + tagsReversed_[tagId]);
					}
					else
					{
						result << ("+ " + tagsReversed_[tagId]);
					}
				}
				return result;
			}
		}

		for (QString tag : tags_.keys())
		{
			result << ("+ " + tag);
		}
		return result;
	}

	void updateCardTags(const QString& card, const QString& update)
	{
		if (update.startsWith('+'))
		{
			auto tagName = update.mid(1).trimmed();
			if (tags_.contains(tagName))
			{
				auto tagId = tags_[tagName];
				if (!cards_.contains(card))
				{
					QSqlDatabase db = conn();
					QSqlQuery q(db);
					if (!q.prepare("INSERT INTO card(name) VALUES(?)")) return;
					q.addBindValue(card);
					if (!q.exec()) return;
					cards_[card] = q.lastInsertId().toInt();
				}
				auto cardId = cards_[card];
				if (!cardTags_[cardId].contains(tagId))
				{
					QSqlDatabase db = conn();
					QSqlQuery q(db);
					if (!q.prepare("INSERT INTO cardtag(tag_id, card_id) VALUES(?, ?)")) return;
					q.addBindValue(tagId);
					q.addBindValue(cardId);
					if (!q.exec()) return;
					cardTags_[cardId].push_back(tagId);
				}
			}
		}
		else
		if (update.startsWith('-'))
		{
			auto tagName = update.mid(1).trimmed();
			if (tags_.contains(tagName))
			{
				auto tagId = tags_[tagName];
				if (cards_.contains(card))
				{
					auto cardId = cards_[card];
					if (cardTags_[cardId].contains(tagId))
					{
						QSqlDatabase db = conn();
						QSqlQuery q(db);
						if (!q.prepare("DELETE FROM cardtag WHERE tag_id = ? AND card_id = ?")) return;
						q.addBindValue(tagId);
						q.addBindValue(cardId);
						if (!q.exec()) return;
						cardTags_[cardId].removeAll(tagId);
					}
				}
			}
		}
	}
};

Tags& Tags::instance()
{
	static Tags inst;
	return inst;
}

QStringList Tags::getTags() const
{
	return pimpl_->getTags();
}

void Tags::addTag(const QString& tag)
{
	pimpl_->addTag(tag);
}

void Tags::removeTag(const QString& tag)
{
	pimpl_->removeTag(tag);
}

QStringList Tags::getCardTags(const QString& card) const
{
	return pimpl_->getCardTags(card);
}

QStringList Tags::getCardCompletions(const QString& card) const
{
	return pimpl_->getCardCompletions(card);
}

void Tags::updateCardTags(const QString& card, const QString& update)
{
	pimpl_->updateCardTags(card, update);
}

Tags::Tags()
	: pimpl_(new Pimpl())
{
}

Tags::~Tags()
{
}
