#pragma once

#include "pooltablemodel.h"
#include "filter.h"
#include "ui_poolwindow.h"

#include <QMainWindow>
#include <QVector>
#include <QScopedPointer>

class PoolWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit PoolWindow(QWidget *parent = 0);
	virtual ~PoolWindow();

	void reload();
	void updateShortcuts();
	void loadSettings();
	void saveSettings();

signals:
	void windowClosed(bool);
	void selectedCardChanged(int);
	void addToCollection(QVector<int>);
	void removeFromCollection(QVector<int>);
	void addToDeck(QVector<int>);
	void removeFromDeck(QVector<int>);
	void fontChanged();

private:
	Ui::PoolWindow ui_;
	PoolTableModel poolTableModel_;
	QScopedPointer<QAbstractItemDelegate> itemDelegate_;
	FilterNode::Ptr rootFilterNode_;

	virtual void closeEvent(QCloseEvent* event);
	virtual bool event(QEvent* event);
	int currentDataRowIndex() const;
	QVector<int> currentDataRowIndices() const;
	void updateStatusBar();

private slots:
	void currentRowChanged(QModelIndex, QModelIndex);
	void actionAdvancedFilter();
	void actionAddToCollection();
	void actionRemoveFromCollection();
	void actionAddToDeck();
	void actionRemoveFromDeck();
	void hideColumnsContextMenuRequested(const QPoint& pos);
	void handleGlobalFilterChanged();
};
