#pragma once

#include <QObject>
#include <QAction>
#include <QMenu>

class CommonActions : public QObject
{
	Q_OBJECT
public:
	explicit CommonActions(QObject *parent = nullptr);

	void updateShortcuts();
	void connectSignals(QObject* object);
	void addToMenu(QMenu* menu);
	void addToWidget(QWidget* widget);

	QAction* getEnableFilter();

private:
	QAction* actionAdvancedFilter;
	QAction* actionEnableFilter;
	QAction* actionAddToCollection;
	QAction* actionRemoveFromCollection;
	QAction* actionAddToDeck;
	QAction* actionRemoveFromDeck;
	QAction* actionDownloadCardArt;
	QAction* actionFetchOnlineData;
};
