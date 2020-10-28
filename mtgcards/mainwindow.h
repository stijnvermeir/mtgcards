#pragma once

#include "ui_mainwindow.h"
#include "pooldock.h"

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

	virtual void closeEvent(QCloseEvent*);

private slots:
	void optionsActionClicked();
	void aboutActionClicked();
	void importDec();
	void globalFilter();
	void onlineManual();
};
