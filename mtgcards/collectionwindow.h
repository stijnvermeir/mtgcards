#pragma once

#include "ui_collectionwindow.h"
#include <QMainWindow>

class CollectionWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit CollectionWindow(QWidget *parent = 0);
	~CollectionWindow();

signals:
	void windowClosed(bool);

private:
	Ui::CollectionWindow ui_;

	void closeEvent(QCloseEvent* event);
};
