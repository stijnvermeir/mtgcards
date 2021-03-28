#pragma once

#include "ui_mainwindow.h"
#include "pooldock.h"
#include "collectiondock.h"
#include "carddock.h"
#include "deckwindow.h"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	void loadSettings();
	void saveSettings();

signals:
	void globalFilterChanged();

private:
	Ui::MainWindow ui_;
	PoolDock* poolDock_;
	CollectionDock* collectionDock_;
	CardDock* cardDock_;
	DeckWindow* deckWindow_;

	virtual void closeEvent(QCloseEvent*);

private slots:
	void optionsActionClicked();
	void aboutActionClicked();
	void globalFilter();
	void updatePrices();
};
