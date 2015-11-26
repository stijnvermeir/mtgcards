#include "filters/tagfilterfunction.h"
#include "magiccarddata.h"
#include "tags.h"

#include <QtWidgets/QComboBox>

namespace {

const QString ID = "Tag";
const bool registered = FilterFunctionManager::instance().registerFilterFunction(ID, []()
{
	return FilterFunction::Ptr(new TagFilterFunction());
});

} // namespace

TagFilterFunction::TagFilterFunction()
	: tag_()
{
}

const QString& TagFilterFunction::getId() const
{
	return ID;
}

bool TagFilterFunction::apply(const QVariant& data) const
{
	if (data.type() == QVariant::Int)
	{
		return mtg::CardData::instance().get(data.toInt(), mtg::ColumnType::Tags).toStringList().contains(tag_);
	}
	return true;
}

QString TagFilterFunction::getDescription() const
{
	return tag_;
}

QJsonObject TagFilterFunction::toJson() const
{
	QJsonObject obj;
	obj["type"] = ID;
	obj["tag"] = tag_;
	return obj;
}

void TagFilterFunction::fromJson(const QJsonObject& obj)
{
	tag_ = obj["tag"].toString();
}

QWidget* TagFilterFunction::createEditor(QWidget* parent) const
{
	QComboBox* tagCbx = new QComboBox(parent);
	tagCbx->addItems(Tags::instance().getTags());
	tagCbx->setCurrentText(tag_);
	return tagCbx;
}

void TagFilterFunction::updateFromEditor(const QWidget* editor)
{
	const QComboBox* tagCbx = static_cast<const QComboBox*>(editor);
	tag_ = tagCbx->currentText();
}

mtg::ColumnType TagFilterFunction::getColumnOverride() const
{
	return mtg::ColumnType::Id;
}
