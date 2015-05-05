#pragma once

#include "collectiontablemodel.h"
#include "filter.h"
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
	CollectionTableModel collectionTableModel_;
	FilterNode::Ptr rootFilterNode_;

	void closeEvent(QCloseEvent* event);

public slots:
	void addToCollection(int);

private slots:
	void hideColumnsContextMenuRequested(const QPoint& pos);
};
