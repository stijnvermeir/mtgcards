#include "filters/commanderfilterfunction.h"
#include "magicconvert.h"
#include "magiccarddata.h"

#include <QtWidgets/QLineEdit>

using namespace std;

namespace {

const QString ID = "Commander";
const bool registered = FilterFunctionManager::instance().registerFilterFunction(ID, []()
{
	return FilterFunction::Ptr(new CommanderFilterFunction());
});

} // namespace

CommanderFilterFunction::CommanderFilterFunction()
    : regex_()
    , input_()
{
}

const QString& CommanderFilterFunction::getId() const
{
	return ID;
}

bool CommanderFilterFunction::apply(const QVariant& data) const
{
	if (data.type() == QVariant::Int)
	{
		if (!mtg::CardData::instance().get(data.toInt(), mtg::ColumnType::CanBeCommander).toBool())
		{
			return false;
		}
		if (mtg::CardData::instance().get(data.toInt(), mtg::ColumnType::LegalityCommander).toString() == "Banned")
		{
			return false;
		}
		auto colId = mtg::toString(mtg::CardData::instance().get(data.toInt(), mtg::ColumnType::ColorIdentity));
		return regex_.match(colId).hasMatch();
	}
	return true;
}

QString CommanderFilterFunction::getDescription() const
{
	return input_;
}

QJsonObject CommanderFilterFunction::toJson() const
{
	QJsonObject obj;
	obj["type"] = ID;
	obj["input"] = input_;
	return obj;
}

void CommanderFilterFunction::fromJson(const QJsonObject& obj)
{
	input_ = obj["input"].toString();
	updateRegex();
}

QWidget* CommanderFilterFunction::createEditor(QWidget* parent) const
{
	QLineEdit* searchTxt = new QLineEdit(parent);
	searchTxt->setText(input_);
	return searchTxt;
}

void CommanderFilterFunction::updateFromEditor(const QWidget* editor)
{
	const QLineEdit* searchTxt = static_cast<const QLineEdit*>(editor);
	input_ = searchTxt->text();
	updateRegex();
}

mtg::ColumnType CommanderFilterFunction::getColumnOverride() const
{
	return mtg::ColumnType::Id;
}

void CommanderFilterFunction::updateRegex()
{
	QString pattern = "^";
	for (QChar c : "WUBRGC")
	{
		if (input_.contains(c, Qt::CaseInsensitive))
		{
			pattern += QString("{%1}").arg(c);
		}
	}
	pattern += "$";
	regex_.setPattern(pattern);
}
