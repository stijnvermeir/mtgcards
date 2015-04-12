#pragma once

#include "ui_cardwindow.h"
#include <QMainWindow>

class CardWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit CardWindow(QWidget *parent = 0);
	~CardWindow();

signals:
	void windowClosed(bool);

private:
	Ui::CardWindow ui_;

	void closeEvent(QCloseEvent* event);

public slots:
	void changeCardPicture(QStringList);
};
