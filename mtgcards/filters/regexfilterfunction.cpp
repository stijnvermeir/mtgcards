#include "filters/regexfilterfunction.h"

#include "magicconvert.h"

#include <QtWidgets/QLineEdit>

namespace {

const QString ID = "Regex";
const bool registered = FilterFunctionManager::instance().registerFilterFunction(ID, []()
{
	return FilterFunction::Ptr(new RegexFilterFunction());
});

} // namespace

RegexFilterFunction::RegexFilterFunction()
	: regex_()
{
}

void RegexFilterFunction::setRegex(const QRegularExpression& regex)
{
	regex_ = regex;
}

const QRegularExpression& RegexFilterFunction::getRegex() const
{
	return regex_;
}

const QString& RegexFilterFunction::getId() const
{
	return ID;
}

bool RegexFilterFunction::apply(const QVariant& data) const
{
	return regex_.match(mtg::toString(data)).hasMatch();
}

QString RegexFilterFunction::getDescription() const
{
	return regex_.pattern();
}

QJsonObject RegexFilterFunction::toJson() const
{
	QJsonObject obj;
	obj["type"] = ID;
	obj["pattern"] = regex_.pattern();
	return obj;
}

void RegexFilterFunction::fromJson(const QJsonObject& obj)
{
	regex_.setPattern(obj["pattern"].toString());
}

QWidget* RegexFilterFunction::createEditor(QWidget* parent) const
{
	QLineEdit* regexTxt = new QLineEdit(parent);
	regexTxt->setText(regex_.pattern());
	return regexTxt;
}

void RegexFilterFunction::updateFromEditor(const QWidget* editor)
{
	const QLineEdit* regexTxt = static_cast<const QLineEdit*>(editor);
	regex_.setPattern(regexTxt->text());
}
