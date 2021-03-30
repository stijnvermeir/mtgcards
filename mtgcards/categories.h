#pragma once

#include <QScopedPointer>
#include <QStringList>

class Categories
{
public:
	static Categories& instance();

	QStringList getCategories() const;
	void addCategory(const QString& category);
	void removeCategory(const QString& category);

private:
	Categories();
	~Categories();

	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
