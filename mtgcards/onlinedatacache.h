#pragma once

#include "magiccolumntype.h"

#include <QScopedPointer>

class OnlineDataCache
{
public:

	static bool isOnlineColumn(const mtg::ColumnType::type_t column);
	static OnlineDataCache& instance();

	const QVariant& get(const QString& set, const QString& name, const mtg::ColumnType& column) const;
	void set(const QString& set, const QString& name, const mtg::ColumnType& column, const QVariant& data);

	void save();
private:
	OnlineDataCache();
	~OnlineDataCache();

	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
