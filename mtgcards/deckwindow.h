#pragma once

#include "ui_deckwindow.h"
#include <QMainWindow>

class DeckWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit DeckWindow(QWidget *parent = 0);
	~DeckWindow();

signals:
	void windowClosed(bool);

private:
	Ui::DeckWindow ui_;

	void closeEvent(QCloseEvent* event);
};
