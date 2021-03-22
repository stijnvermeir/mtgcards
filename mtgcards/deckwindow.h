#pragma once

#include "filter.h"
#include "commonactions.h"
#include "ui_mainwindow.h"

#include <QObject>
#include <QVector>
#include <QString>
#include <QToolBar>

class DeckWidget;
class QLabel;

class DeckWindow : public QObject
{
	Q_OBJECT

public:
	explicit DeckWindow(Ui::MainWindow& ui, QWidget* parent = 0);
	virtual ~DeckWindow();

	void updateShortcuts();
	void loadSettings();
	void saveSettings();
	bool hasUnsavedChanges() const;
	void openDeck(const QString& deckId);

signals:
	void selectedCardChanged(int);
	void addToCollection(QVector<int>);
	void addToCollection(QVector<QPair<int,int>>);
	void removeFromCollection(QVector<int>);
	void fontChanged();

private:
	Ui::MainWindow& ui_;
	QString headerState_;
	FilterNode::Ptr rootFilterNode_;
	QAction* actionNewDeck_;
	QAction* actionOpenDeck_;
	QAction* actionSaveDeck_;
	QAction* actionSaveDeckAs_;
	QAction* actionImportDec_;
	QAction* actionToggleDeckActive_;
	QAction* actionAddDeckToCollection_;
	QAction* actionCreateProxies_;
	QAction* actionStats_;
	QAction* actionCopyDeckStatsClipboard_;
	CommonActions commonActions_;
	QToolBar* toolBar_;

	void updateStatusBar();
	DeckWidget* createDeckWidget(const QString& filename = QString());
	void destroyDeckWidget(DeckWidget* deckWidget);
	void saveDeck(DeckWidget* deckWidget, bool saveAs);

public slots:
	void addToDeck(const QVector<int>&);
	void removeFromDeck(const QVector<int>&);

private slots:
	void selectedCardChangedSlot();
	void closeDeck(int);
	void currentTabChangedSlot(int);
	void actionNewDeck();
	void actionOpenDeck();
	void actionSaveDeck();
	void actionSaveDeckAs();
	void actionImportDec();
	void actionAdvancedFilter();
	void actionEnableFilter(bool enable);
	void actionAddToCollection();
	void actionAddDeckToCollection();
	void actionRemoveFromCollection();
	void actionAddToDeck();
	void actionRemoveFromDeck();
	void actionToggleDeckActive(bool);
	void createProxies();
	void showStatistics();
	void copyDeckstatsClipboard();
	void actionDownloadCardArt();
	void actionFetchOnlineData();
	void deckEdited();
	void handleOpenDeckRequest(const QString& deckId);
	void headerStateChangedSlot(const QString& headerState);
	void handleGlobalFilterChanged();
};
