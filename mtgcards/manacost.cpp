#include "manacost.h"

#include <QFontDatabase>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>

#include <map>

using namespace std;

namespace
{

const map<QChar, QString> symbolToColorMap =
{
	{'W', "white"},
	{'U', "blue"},
	{'B', "black"},
	{'R', "red"},
	{'G', "green"}
};

QString convertSingleManaSymbol(const QChar symbol)
{
	QString result = "<span class='symbol'>";
	result += "<span class='border'>O</span>";
	result += "<span class='" + symbolToColorMap.at(symbol) + "'>o</span>";
	result += "<span class='text'>";
	result += symbol.toLower();
	result += "</span>";
	result += "</span>";
	return result;
}

} // namespace

QString ManaCost::getRichText() const
{
	static bool loadStaticResources = true;
	static QString styleSheetContent = "";
	if (loadStaticResources)
	{
		QFontDatabase::addApplicationFont(":/resources/fonts/MagicSymbols2013.ttf");

		QFile styleFile(":/resources/fonts/style.css");
		styleFile.open(QFile::ReadOnly | QFile::Text);
		QTextStream in(&styleFile);
		styleSheetContent = in.readAll();

		loadStaticResources = false;
	}

	QString text = "<html><style type=\"text/css\">" + styleSheetContent + "</style><body>";
	QString copy = text_;
	QRegularExpression re("\\{[WUBRG]\\}");
	QRegularExpressionMatchIterator i = re.globalMatch(text_);
	while (i.hasNext())
	{
		QRegularExpressionMatch match = i.next();
		QString cap = match.captured();
		copy.replace(cap, convertSingleManaSymbol(cap[1]));
	}
	text += copy;
	text += "</body></html>";
	return text;
}
