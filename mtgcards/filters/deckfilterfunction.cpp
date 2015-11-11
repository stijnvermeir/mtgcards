#include "filters/deckfilterfunction.h"

#include "deckmanager.h"

#include <QtWidgets/QComboBox>

using namespace std;

namespace {

const QString ID = "Deck";
const bool registered = FilterFunctionManager::instance().registerFilterFunction(ID, []()
{
	return FilterFunction::Ptr(new DeckFilterFunction());
});

} // namespace

DeckFilterFunction::DeckFilterFunction()
	: deck_()
{
}

const QString& DeckFilterFunction::getId() const
{
	return ID;
}

bool DeckFilterFunction::apply(const QVariant& data) const
{
	if (data.type() == QVariant::Int && deck_)
	{
		return deck_.data()->getRowIndex(data.toInt()) != -1;
	}
	return true;
}

QString DeckFilterFunction::getDescription() const
{
	if (deck_)
	{
		return deck_.data()->getDisplayName();
	}
	return "No deck";
}

QJsonObject DeckFilterFunction::toJson() const
{
	QJsonObject obj;
	obj["type"] = ID;
	if (deck_)
	{
		obj["deck"] = deck_.data()->getFilename();
	}
	else
	{
		obj["deck"] = QString("");
	}
	return obj;
}

void DeckFilterFunction::fromJson(const QJsonObject& obj)
{
	deck_ = DeckManager::instance().findDeck(obj["deck"].toString());
}

QWidget* DeckFilterFunction::createEditor(QWidget* parent) const
{
	QComboBox* cbx = new QComboBox(parent);
	for (const QSharedPointer<Deck>& deck : DeckManager::instance().getDecks())
	{
		cbx->addItem(deck->getDisplayName(), deck->getId());
	}
	if (deck_)
	{
		cbx->setCurrentText(deck_.data()->getDisplayName());
	}
	return cbx;
}

void DeckFilterFunction::updateFromEditor(const QWidget* editor)
{
	const QComboBox* cbx = static_cast<const QComboBox*>(editor);
	deck_ = DeckManager::instance().findDeck(cbx->currentData().toString()).toWeakRef();
}

