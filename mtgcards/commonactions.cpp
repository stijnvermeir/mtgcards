#include "commonactions.h"

#include "settings.h"

namespace  {
    QIcon createIcon(const QString& path)
	{
		return QIcon(path);
	}

	QIcon createIcon(const QString& pathOff, const QString& pathOn)
	{
		QIcon icon;
		icon.addFile(pathOff, QSize(), QIcon::Normal, QIcon::Off);
		icon.addFile(pathOn, QSize(), QIcon::Normal, QIcon::On);
		return icon;
	}
}

CommonActions::CommonActions(QObject *parent) : QObject(parent)
{
	actionAdvancedFilter = new QAction(createIcon(":/resources/icons/funnel2.svg"), "Advanced filter", this);
	actionAdvancedFilter->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionEnableFilter = new QAction(createIcon(":/resources/icons/switches3.svg", ":/resources/icons/switch14.svg"), "Enable filter", this);
	actionEnableFilter->setCheckable(true);
	actionEnableFilter->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionAddToCollection = new QAction(createIcon(":/resources/icons/add182.svg"), "Add to collection", this);
	actionAddToCollection->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionRemoveFromCollection = new QAction(createIcon(":/resources/icons/rounded56.svg"), "Remove from collection", this);
	actionRemoveFromCollection->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionAddToDeck = new QAction(createIcon(":/resources/icons/rounded54.svg"), "Add to deck", this);
	actionAddToDeck->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionRemoveFromDeck = new QAction(createIcon(":/resources/icons/round56.svg"), "Remove from deck", this);
	actionRemoveFromDeck->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionDownloadCardArt = new QAction(createIcon(":/resources/icons/paint-board-and-brush.svg"), "Download card art", this);
	actionDownloadCardArt->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	actionFetchOnlineData = new QAction(createIcon(":/resources/icons/internet43.svg"), "Fetch online data", this);
	actionFetchOnlineData->setShortcutContext(Qt::WidgetWithChildrenShortcut);
}

void CommonActions::updateShortcuts()
{
	actionAdvancedFilter->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AdvancedFilter]);
	actionEnableFilter->setShortcut(Settings::instance().getShortcuts()[ShortcutType::EnableFilter]);
	actionAddToCollection->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AddToCollection]);
	actionRemoveFromCollection->setShortcut(Settings::instance().getShortcuts()[ShortcutType::RemoveFromCollection]);
	actionAddToDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AddToDeck]);
	actionRemoveFromDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::RemoveFromDeck]);
	actionDownloadCardArt->setShortcut(Settings::instance().getShortcuts()[ShortcutType::DownloadCardArt]);
	actionFetchOnlineData->setShortcut(Settings::instance().getShortcuts()[ShortcutType::FetchOnlineData]);
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
