#pragma once

#include "magiclayouttype.h"
#include "pooltablemodel.h"
#include "filter.h"
#include "ui_poolwindow.h"

#include <QMainWindow>

class PoolWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit PoolWindow(QWidget *parent = 0);
	~PoolWindow();

	void reload();

	void loadSettings();
	void saveSettings();

signals:
	void windowClosed(bool);
	void selectCardChanged(mtg::LayoutType layout, QStringList);

private:
	Ui::PoolWindow ui_;
	PoolTableModel poolTableModel_;
	FilterNode::Ptr rootFilterNode_;

	void closeEvent(QCloseEvent* event);

private slots:
	void currentRowChanged(QModelIndex, QModelIndex);
	void actionAdvancedFilter();
};
