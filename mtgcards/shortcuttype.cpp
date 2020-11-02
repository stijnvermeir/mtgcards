#include "shortcuttype.h"

namespace {

const QVector<QString> NAMES =
{
	 "newfile",
     "importfile",
	 "openfile",
	 "savefile",
	 "savefileas",
	 "advancedfilter",
     "enablefilter",
	 "addtocollection",
	 "removefromcollection",
	 "addtodeck",
	 "removefromdeck",
	 "addfiltergroup",
	 "addfilternode",
     "removefilternode",
     "downloadcardart",
     "fetchonlinedata"
};

const QVector<QString> DESCRIPTIONS =
{
	 "New File",
     "Import File",
	 "Open File",
	 "Save File",
	 "Save File as",
	 "Advanced Filter",
     "Enable Filter",
	 "Add To Collection",
	 "Remove From Collection",
	 "Add To Deck",
	 "Remove From Deck",
	 "Add Filter Group",
	 "Add Filter Node",
     "Remove Filter Node",
     "Download card art",
     "Fetch online data"
};

QVector<QKeySequence> DEFAULT_KEY_SEQUENCES;
void initDefaultKeySequences()
{
	static bool initialized = false;
	if (!initialized)
	{
		DEFAULT_KEY_SEQUENCES.reserve(ShortcutType::COUNT);
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence::New);
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence("CTRL+I"));
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence::Open),
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence::Save);
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence::SaveAs);
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence("CTRL+F"));
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence("CTRL+SHIFT+F"));
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence("CTRL+ALT+="));
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence("CTRL+ALT+-"));
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence("CTRL+="));
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence("CTRL+-"));
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence("CTRL+G"));
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence("CTRL+F"));
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence::Delete);
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence("CTRL+D"));
		DEFAULT_KEY_SEQUENCES.push_back(QKeySequence("CTRL+U"));
		initialized = true;
	}
}

} // namespace

ShortcutType::ShortcutType(const type_t value)
	: value_(value)
{
}

ShortcutType::ShortcutType(const int value)
	: value_(UNKNOWN)
{
	if (value >= 0 && value < COUNT)
	{
		value_ = static_cast<type_t>(value);
	}
}

ShortcutType::ShortcutType(const QString& stringValue)
	: ShortcutType(NAMES.indexOf(stringValue))
{
}

ShortcutType::operator QString () const
{
	if (value_ >= 0 && value_ < COUNT)
	{
		return NAMES[value_];
	}
	return "UNKNOWN";
}

ShortcutType::operator type_t () const
{
	return value_;
}

bool ShortcutType::operator <(const ShortcutType& other) const
{
	return value_ < other.value_;
}

const QString& ShortcutType::getDescription() const
{
	if (value_ >= 0 && value_ < COUNT)
	{
		return DESCRIPTIONS[value_];
	}
	static const QString EMPTY = "";
	return EMPTY;
}

const QKeySequence& ShortcutType::getDefaultKeySequence() const
{
	initDefaultKeySequences();
	if (value_ >= 0 && value_ < COUNT)
	{
		return DEFAULT_KEY_SEQUENCES[value_];
	}
	static const QKeySequence EMPTY;
	return EMPTY;
}

const QVector<ShortcutType>& ShortcutType::list()
{
	static QVector<ShortcutType> l;
	static bool ready = false;
	if (!ready)
	{
		l.reserve(COUNT);
		for (int i = 0; i < COUNT; ++i)
		{
			l.push_back(ShortcutType(static_cast<ShortcutType::type_t>(i)));
		}
		ready = true;
	}
	return l;
}
