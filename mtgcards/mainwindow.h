#pragma once

#include "poolwindow.h"
#include "cardwindow.h"
#include "collectionwindow.h"
#include "deckwindow.h"
#include "ui_mainwindow.h"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	Ui::MainWindow ui_;
	PoolWindow poolWindow_;
	CardWindow cardWindow_;
	CollectionWindow collectionWindow_;
	DeckWindow deckWindow_;

	void closeEvent(QCloseEvent* event);

private slots:
	void poolWindowActionToggled(bool show);
	void cardWindowActionToggled(bool show);
	void collectionWindowActionToggled(bool show);
	void deckWindowActionToggled(bool show);
};
