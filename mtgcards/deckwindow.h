#pragma once

#include "filter.h"
#include "ui_deckwindow.h"

#include <QMainWindow>

class DeckWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit DeckWindow(QWidget *parent = 0);
	~DeckWindow();

	void updateShortcuts();
	void loadSettings();
	void saveSettings();

signals:
	void windowClosed(bool);

private:
	Ui::DeckWindow ui_;
	FilterNode::Ptr rootFilterNode_;

	void closeEvent(QCloseEvent* event);
	void updateStatusBar();
};
