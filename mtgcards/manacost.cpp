#include "manacost.h"

#include <QFontDatabase>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QDebug>

#include <map>

using namespace std;

namespace
{

const map<QString, pair<QChar, QString>> simpleSymbolMap =
{
	{"W",  {'w', "white"}},
	{"U",  {'u', "blue"}},
	{"B",  {'b', "black"}},
	{"R",  {'r', "red"}},
	{"G",  {'g', "green"}},
	{"S",  {'s', "gray"}},
	{"X",  {'x', "gray"}},
	{"Y",  {'y', "gray"}},
	{"Z",  {'z', "gray"}},
	{"0",  {'0', "gray"}},
	{"1",  {'1', "gray"}},
	{"2",  {'2', "gray"}},
	{"3",  {'3', "gray"}},
	{"4",  {'4', "gray"}},
	{"5",  {'5', "gray"}},
	{"6",  {'6', "gray"}},
	{"7",  {'7', "gray"}},
	{"8",  {'8', "gray"}},
	{"9",  {'9', "gray"}},
	{"10", {'A', "gray"}},
	{"11", {'D', "gray"}},
	{"12", {'F', "gray"}},
	{"13", {'H', "gray"}},
	{"14", {'K', "gray"}},
	{"15", {'L', "gray"}},
	{"16", {'M', "gray"}},
	{"17", {'N', "gray"}},
	{"18", {'V', "gray"}},
	{"19", {'Y', "gray"}},
	{"20", {'Z', "gray"}}
};

QString convertSimpleSymbol(const QString& symbol)
{
	QString result = "<span class='symbol'>";
	result += "<span class='border'>O</span>";
	result += QString("<span class='%1'>o</span>").arg(simpleSymbolMap.at(symbol).second);
	result += QString("<span class='text'>%1</span>").arg(simpleSymbolMap.at(symbol).first);
	result += "</span>";
	return result;
}

const map<QChar, QString> phyrexianSymbolMap =
{
	{'W', "darkwhite"},
	{'U', "darkblue"},
	{'B', "darkblack"},
	{'R', "darkred"},
	{'G', "darkgreen"}
};

QString convertPhyrexianSymbol(const QChar& symbol)
{
	QString result = "<span class='symbol'>";
	result += "<span class='border'>C</span>";
	result += "<span class='" + phyrexianSymbolMap.at(symbol) + "'>c</span>";
	result += "<span class='text'>P</span>";
	result += "</span>";
	return result;
}

struct CompositeSymbolInfo
{
	QString upperChar;
	QString lowerChar;
	QString color;
};

const map<QChar, CompositeSymbolInfo> compositeSymbolMap =
{
	{'W', {"W",     "a", "white"}},
	{'U', {"U",     "d", "blue"}},
	{'B', {"B",     "f", "black"}},
	{'R', {"R",     "k", "red"}},
	{'G', {"G",     "l", "green"}},
	{'0', {")",     " ", "gray"}},
	{'2', {"@",     " ", "gray"}},
	{'3', {"#",     " ", "gray"}},
	{'4', {"$",     " ", "gray"}},
	{'5', {"%",     " ", "gray"}},
	{'6', {"^",     " ", "gray"}},
	{'7', {"&amp;", " ", "gray"}},
	{'8', {"*",     " ", "gray"}},
	{'9', {"(",     " ", "gray"}}
};

QString convertCompositeSymbol(const QChar& upper, const QChar& lower)
{
	QString result = "<span class='symbol'>";
	result += "<span class='border'>C</span>";
	result += QString("<span class='%1'>[</span>").arg(compositeSymbolMap.at(upper).color);
	result += QString("<span class='text'>%1</span>").arg(compositeSymbolMap.at(upper).upperChar);
	result += QString("<span class='%1'>/</span>").arg(compositeSymbolMap.at(lower).color);
	result += QString("<span class='text'>%1</span>").arg(compositeSymbolMap.at(lower).lowerChar);
	result += "</span>";
	return result;
}

const map<QChar, pair<QChar, QString>> halfSymbolMap =
{
	{'w', {'\'', "white"}},
	{'u', {'=', "blue"}},
	{'b', {';', "black"}},
	{'r', {':', "red"}},
	{'g', {'_', "green"}}
};

QString convertHalfSymbol(const QChar& symbol)
{
	QString result = "<span class='symbol'>";
	result += "<span class='border'>|</span>";
	result += QString("<span class='%1'>\\</span>").arg(halfSymbolMap.at(symbol).second);
	result += QString("<span class='text'>%1</span>").arg(halfSymbolMap.at(symbol).first);
	result += "</span>";
	return result;
}

QString convertMillionSymbol()
{
	QString result = "<span class='symbol'>";
	result += "<span class='border'>&lt;</span>";
	result += "<span class='gray'>,</span>";
	result += "<span class='text'>m</span>";
	result += "</span>";
	return result;
}

map<QString, QString> tagToRichTextMap;
void initializeTagToRichTextMap()
{
	// simple symbols
	for (const auto& c : QString("WUBRGSXYZ"))
	{
		tagToRichTextMap[QString("{%1}").arg(c)] = convertSimpleSymbol(c);
	}
	// numbers
	for (int i = 0; i <= 20; ++i)
	{
		tagToRichTextMap[QString("{%1}").arg(i)] = convertSimpleSymbol(QString::number(i));
	}
	// phyrexian symbols
	for (const auto& c : QString("WUBRG"))
	{
		tagToRichTextMap[QString("{%1/P}").arg(c)] = convertPhyrexianSymbol(c);
	}
	// composite symbols
	for (const auto& u : QString("2WUBRG"))
	{
		for (const auto& l : QString("WUBRG"))
		{
			tagToRichTextMap[QString("{%1/%2}").arg(u, l)] = convertCompositeSymbol(u, l);
		}
	}
	// half symbols
	for (const auto& c : QString("wubrg"))
	{
		tagToRichTextMap[QString("{h%1}").arg(c)] = convertHalfSymbol(c);
	}
	// special symbols
	tagToRichTextMap["{1000000}"] = convertMillionSymbol();
}

} // namespace

QString ManaCost::getRichText() const
{
	static bool loadStaticResources = true;
	static QString prefix = "";
	static QString suffix = "</body></html>";
	if (loadStaticResources)
	{
		// load font
		QFontDatabase::addApplicationFont(":/resources/fonts/MagicSymbols2013.ttf");

		// load style sheet
		QFile styleFile(":/resources/fonts/style.css");
		styleFile.open(QFile::ReadOnly | QFile::Text);
		QTextStream in(&styleFile);
		prefix = "<html><style type=\"text/css\">" + in.readAll() + "</style><body>";

		// initialize map
		initializeTagToRichTextMap();

		loadStaticResources = false;
	}

	QString copy = text_;
	for (const auto& mapping : tagToRichTextMap)
	{
		copy.replace(mapping.first, mapping.second);
		if (!copy.contains('{'))
		{
			// don't continue trying to replace if it doesn't make sense anymore
			break;
		}
	}
	if (copy.contains('{'))
	{
		qWarning() << "Found an unreplaced bracket: " << text_;
	}
	return prefix + copy + suffix;
}

bool operator<(const ManaCost& lhs, const ManaCost& rhs)
{
	if (lhs.getCmc() == rhs.getCmc())
	{
		return lhs.getText() < rhs.getText();
	}
	return lhs.getCmc() < rhs.getCmc();
}
