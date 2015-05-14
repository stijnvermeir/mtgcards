#pragma once

#include <QString>
#include <QKeySequence>
#include <QVector>

class ShortcutType
{
public:
	enum type_t
	{
		NewFile,
		OpenFile,
		SaveFile,
		SaveFileAs,
		AdvancedFilter,
		AddToCollection,
		RemoveFromCollection,
		AddToDeck,
		RemoveFromDeck,
		AddFilterGroup,
		AddFilterNode,
		RemoveFilterNode,

		COUNT,
		UNKNOWN = -1
	};

	ShortcutType(const type_t value = UNKNOWN);
	ShortcutType(const int value);
	ShortcutType(const QString& stringValue);

	operator QString () const;
	operator type_t () const;
	bool operator <(const ShortcutType& other) const;

	const QString& getDescription() const;
	const QKeySequence& getDefaultKeySequence() const;

	static const QVector<ShortcutType>& list();

private:
	type_t value_;
};
