#pragma once

#include "enum.h"
#include "pooltablemodel.h"
#include "ui_poolwindow.h"
#include <QMainWindow>

class PoolWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit PoolWindow(QWidget *parent = 0);
	~PoolWindow();

	void reload();

signals:
	void windowClosed(bool);
	void selectCardChanged(layout_type_t layout, QStringList);

private:
	Ui::PoolWindow ui_;
	PoolTableModel poolTableModel_;

	void closeEvent(QCloseEvent* event);

private slots:
	void currentRowChanged(QModelIndex, QModelIndex);
	void actionAdvancedFilterToggled(bool);
};
