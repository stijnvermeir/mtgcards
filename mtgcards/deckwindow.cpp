#include "deckwindow.h"

#include "settings.h"

#include <QCloseEvent>
#include <QSettings>
#include <QDebug>

DeckWindow::DeckWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, rootFilterNode_()
{
	ui_.setupUi(this);
}

DeckWindow::~DeckWindow()
{
}

void DeckWindow::updateShortcuts()
{
	ui_.actionAdvancedFilter->setShortcut(Settings::instance().getShortcuts().at(ShortcutType::AdvancedFilter));
	ui_.actionAddToCollection->setShortcut(Settings::instance().getShortcuts().at(ShortcutType::AddToCollection));
	ui_.actionRemoveFromCollection->setShortcut(Settings::instance().getShortcuts().at(ShortcutType::RemoveFromCollection));
	ui_.actionAddToDeck->setShortcut(Settings::instance().getShortcuts().at(ShortcutType::AddToDeck));
	ui_.actionRemoveFromDeck->setShortcut(Settings::instance().getShortcuts().at(ShortcutType::RemoveFromDeck));
}

void DeckWindow::loadSettings()
{
	QSettings settings;
	if (settings.contains("deckwindow/filter"))
	{
		rootFilterNode_ = FilterNode::createFromJson(QJsonDocument::fromJson(settings.value("deckwindow/filter").toByteArray()));
	}
	updateStatusBar();
	updateShortcuts();
}

void DeckWindow::saveSettings()
{
	QSettings settings;
	if (rootFilterNode_)
	{
		settings.setValue("deckwindow/filter", rootFilterNode_->toJson().toJson());
	}
	else
	{
		settings.remove("deckwindow/filter");
	}
}

void DeckWindow::closeEvent(QCloseEvent* event)
{
	emit windowClosed(false);
	event->accept();
}

void DeckWindow::updateStatusBar()
{

}
