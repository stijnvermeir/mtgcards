#pragma once

#include "filter.h"
#include "ui_deckwindow.h"

#include <QMainWindow>
#include <QVector>
#include <QString>

class DeckWidget;
class QLabel;

class DeckWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit DeckWindow(QWidget* parent = 0);
	virtual ~DeckWindow();

	void reload();
	void updateShortcuts();
	void loadSettings();
	void saveSettings();
	bool hasUnsavedChanges() const;
	void openDeck(const QString& deckId);

signals:
	void windowClosed(bool);
	void selectedCardChanged(int);
	void addToCollection(QVector<int>);
	void removeFromCollection(QVector<int>);
	void fontChanged();

private:
	Ui::DeckWindow ui_;
	QString headerState_;
	FilterNode::Ptr rootFilterNode_;
	QLabel* permanentStatusBarLabel_;

	void closeEvent(QCloseEvent* event);
	virtual bool event(QEvent* event);
	void updateStatusBar();
	DeckWidget* createDeckWidget(const QString& filename = QString::null);
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
	void actionAdvancedFilter();
	void actionEnableFilter(bool enable);
	void actionAddToCollection();
	void actionRemoveFromCollection();
	void actionAddToDeck();
	void actionRemoveFromDeck();
	void actionToggleDeckActive(bool);
	void createProxies();
	void showStatistics();
	void fetchOnlineData();
	void addToWantslist();
	void deckEdited();
	void handleOpenDeckRequest(const QString& deckId);
	void headerStateChangedSlot(const QString& headerState);
	void handleGlobalFilterChanged();
};
