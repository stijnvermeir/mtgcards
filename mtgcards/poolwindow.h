#pragma once

#include "pooltablemodel.h"
#include "ui_poolwindow.h"
#include <QMainWindow>

class PoolWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit PoolWindow(QWidget *parent = 0);
	~PoolWindow();

signals:
	void windowClosed(bool);

private:
	Ui::PoolWindow ui_;
	PoolTableModel poolTableModel_;

	void closeEvent(QCloseEvent* event);
};
