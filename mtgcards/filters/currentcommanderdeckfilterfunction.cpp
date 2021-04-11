#include "filters/currentcommanderdeckfilterfunction.h"
#include "deckmanager.h"
#include "magicconvert.h"
#include "magiccarddata.h"

using namespace std;

namespace {

const QString ID = "Current Commander";
const bool registered = FilterFunctionManager::instance().registerFilterFunction(ID, []()
{
	return FilterFunction::Ptr(new CurrentCommanderDeckFilterFunction());
});

} // namespace

CurrentCommanderDeckFilterFunction::CurrentCommanderDeckFilterFunction()
{
}

const QString& CurrentCommanderDeckFilterFunction::getId() const
{
	return ID;
}

bool CurrentCommanderDeckFilterFunction::apply(const QVariant& data) const
{
	if (data.type() == QVariant::Int)
	{
		auto deck = DeckManager::instance().getCurrentDeck();
		if (deck)
		{
			auto colId = mtg::toString(mtg::CardData::instance().get(data.toInt(), mtg::ColumnType::ColorIdentity));
			return mtg::CardData::instance().get(data.toInt(), mtg::ColumnType::LegalityCommander).toBool() && deck->matchesColorIdentity(colId);
		}
	}
	return true;
}

QString CurrentCommanderDeckFilterFunction::getDescription() const
{
	return "Cards legal for current commander";
}

QJsonObject CurrentCommanderDeckFilterFunction::toJson() const
{
	QJsonObject obj;
	obj["type"] = ID;
	return obj;
}

void CurrentCommanderDeckFilterFunction::fromJson(const QJsonObject&)
{
}

QWidget* CurrentCommanderDeckFilterFunction::createEditor(QWidget*) const
{
	return nullptr;
}

void CurrentCommanderDeckFilterFunction::updateFromEditor(const QWidget*)
{
}

mtg::ColumnType CurrentCommanderDeckFilterFunction::getColumnOverride() const
{
	return mtg::ColumnType::Id;
}

