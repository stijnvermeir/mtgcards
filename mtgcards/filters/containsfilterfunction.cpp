#include "filters/containsfilterfunction.h"

#include "magicconvert.h"

#include <QtWidgets/QLineEdit>

namespace {

const QString ID = "Contains";
const bool registered = FilterFunctionManager::instance().registerFilterFunction(ID, []()
{
	return FilterFunction::Ptr(new ContainsFilterFunction());
});

} // namespace

ContainsFilterFunction::ContainsFilterFunction()
	: search_()
{
}

const QString& ContainsFilterFunction::getId() const
{
	return ID;
}

bool ContainsFilterFunction::apply(const QVariant& data) const
{
	return mtg::toString(data).contains(search_, Qt::CaseInsensitive);
}

QString ContainsFilterFunction::getDescription() const
{
	return search_;
}

QJsonObject ContainsFilterFunction::toJson() const
{
	QJsonObject obj;
	obj["type"] = ID;
	obj["search"] = search_;
	return obj;
}

void ContainsFilterFunction::fromJson(const QJsonObject& obj)
{
	search_ = obj["search"].toString();
}

QWidget* ContainsFilterFunction::createEditor(QWidget* parent) const
{
	QLineEdit* searchTxt = new QLineEdit(parent);
	searchTxt->setText(search_);
	return searchTxt;
}

void ContainsFilterFunction::updateFromEditor(const QWidget* editor)
{
	const QLineEdit* searchTxt = static_cast<const QLineEdit*>(editor);
	search_ = searchTxt->text();
}

