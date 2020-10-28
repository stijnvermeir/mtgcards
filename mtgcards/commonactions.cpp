#include "commonactions.h"

#include "settings.h"

CommonActions::CommonActions(QObject *parent) : QObject(parent)
{
	actionAdvancedFilter = new QAction("Advanced filter", this);
	actionAdvancedFilter->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionEnableFilter = new QAction("Enable filter", this);
	actionEnableFilter->setCheckable(true);
	actionEnableFilter->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionAddToCollection = new QAction("Add to collection", this);
	actionAddToCollection->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionRemoveFromCollection = new QAction("Remove from collection", this);
	actionRemoveFromCollection->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionAddToDeck = new QAction("Add to deck", this);
	actionAddToDeck->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionRemoveFromDeck = new QAction("Remove from deck", this);
	actionRemoveFromDeck->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionDownloadCardArt = new QAction("Download card art", this);
	actionDownloadCardArt->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionFetchOnlineData = new QAction("Fetch online data", this);
	actionFetchOnlineData->setShortcutContext(Qt::WidgetWithChildrenShortcut);
}

void CommonActions::updateShortcuts()
{
	actionAdvancedFilter->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AdvancedFilter]);
	actionAddToCollection->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AddToCollection]);
	actionRemoveFromCollection->setShortcut(Settings::instance().getShortcuts()[ShortcutType::RemoveFromCollection]);
	actionAddToDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AddToDeck]);
	actionRemoveFromDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::RemoveFromDeck]);
}

void CommonActions::connectSignals(QObject* object)
{
	connect(actionAdvancedFilter, SIGNAL(triggered()), object, SLOT(actionAdvancedFilter()));
	connect(actionEnableFilter, SIGNAL(triggered(bool)), object, SLOT(actionEnableFilter(bool)));
	connect(actionAddToCollection, SIGNAL(triggered()), object, SLOT(actionAddToCollection()));
	connect(actionRemoveFromCollection, SIGNAL(triggered()), object, SLOT(actionRemoveFromCollection()));
	connect(actionAddToDeck, SIGNAL(triggered()), object, SLOT(actionAddToDeck()));
	connect(actionRemoveFromDeck, SIGNAL(triggered()), object, SLOT(actionRemoveFromDeck()));
	connect(actionDownloadCardArt, SIGNAL(triggered()), object, SLOT(actionDownloadCardArt()));
	connect(actionFetchOnlineData, SIGNAL(triggered()), object, SLOT(actionFetchOnlineData()));
}

void CommonActions::addToMenu(QMenu* menu)
{
	menu->addAction(actionAdvancedFilter);
	menu->addAction(actionEnableFilter);
	menu->addSeparator();
	menu->addAction(actionAddToCollection);
	menu->addAction(actionRemoveFromCollection);
	menu->addAction(actionAddToDeck);
	menu->addAction(actionRemoveFromDeck);
	menu->addSeparator();
	menu->addAction(actionDownloadCardArt);
	menu->addAction(actionFetchOnlineData);
}

void CommonActions::addToWidget(QWidget* widget)
{
	widget->addAction(actionAdvancedFilter);
	widget->addAction(actionEnableFilter);
	widget->addAction(actionAddToCollection);
	widget->addAction(actionRemoveFromCollection);
	widget->addAction(actionAddToDeck);
	widget->addAction(actionRemoveFromDeck);
	widget->addAction(actionDownloadCardArt);
	widget->addAction(actionFetchOnlineData);
}

QAction* CommonActions::getEnableFilter()
{
	return actionEnableFilter;
}
