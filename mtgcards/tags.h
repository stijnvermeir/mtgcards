#pragma once

#include <QScopedPointer>
#include <QStringList>

class Tags
{
public:
	static Tags& instance();

	QStringList getTags() const;
	void addTag(const QString& tag);
	void removeTag(const QString& tag);

	QStringList getCardTags(const QString& card) const;
	QStringList getCardCompletions(const QString& card) const;
	void updateCardTags(const QString& card, const QString& update);

private:
	Tags();
	~Tags();

	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
