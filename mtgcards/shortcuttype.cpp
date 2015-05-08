#include "shortcuttype.h"

#include <array>
#include <algorithm>

using namespace std;

namespace {

const array<QString, ShortcutType::COUNT> NAMES =
{{
	 "newfile",
	 "openfile",
	 "savefile",
	 "savefileas",
	 "advancedfilter",
	 "addtocollection",
	 "removefromcollection",
	 "addtodeck",
	 "removefromdeck",
	 "addfiltergroup",
	 "addfilternode",
	 "removefilternode"
}};

const array<QString, ShortcutType::COUNT> DESCRIPTIONS =
{{
	 "New File",
	 "Open File",
	 "Save File",
	 "Save File as",
	 "Advanced Filter",
	 "Add To Collection",
	 "Remove From Collection",
	 "Add To Deck",
	 "Remove From Deck",
	 "Add Filter Group",
	 "Add Filter Node",
	 "Remove Filter Node"
}};

vector<QKeySequence> DEFAULT_KEY_SEQUENCES;
void initDefaultKeySequences()
{
	static bool initialized = false;
	if (!initialized)
	{
		DEFAULT_KEY_SEQUENCES.reserve(ShortcutType::COUNT);
		DEFAULT_KEY_SEQUENCES.emplace_back(QKeySequence::New);
		DEFAULT_KEY_SEQUENCES.emplace_back(QKeySequence::Open),
		DEFAULT_KEY_SEQUENCES.emplace_back(QKeySequence::Save);
		DEFAULT_KEY_SEQUENCES.emplace_back(QKeySequence::SaveAs);
		DEFAULT_KEY_SEQUENCES.emplace_back("CTRL+F");
		DEFAULT_KEY_SEQUENCES.emplace_back("CTRL++");
		DEFAULT_KEY_SEQUENCES.emplace_back("CTRL+-");
		DEFAULT_KEY_SEQUENCES.emplace_back("+");
		DEFAULT_KEY_SEQUENCES.emplace_back("-");
		DEFAULT_KEY_SEQUENCES.emplace_back("CTRL+G");
		DEFAULT_KEY_SEQUENCES.emplace_back("CTRL+F");
		DEFAULT_KEY_SEQUENCES.emplace_back(QKeySequence::Delete);
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
	: value_(UNKNOWN)
{
	auto it = find(NAMES.begin(), NAMES.end(), stringValue);
	if (it != NAMES.end())
	{
		value_ = static_cast<type_t>(it - NAMES.begin());
	}
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

const vector<ShortcutType>& ShortcutType::list()
{
	static vector<ShortcutType> l;
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
