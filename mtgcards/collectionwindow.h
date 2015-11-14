#pragma once

#include "collectiontablemodel.h"
#include "filter.h"
#include "ui_collectionwindow.h"

#include <QMainWindow>
#include <QVector>
#include <QScopedPointer>

class QAbstractItemDelegate;

class CollectionWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit CollectionWindow(QWidget* parent = 0);
	virtual ~CollectionWindow();

	void reload();
	void updateShortcuts();
	void loadSettings();
	void saveSettings();

signals:
	void windowClosed(bool);
	void selectedCardChanged(int);
	void addToDeck(QVector<int>);
	void removeFromDeck(QVector<int>);
	void requestOpenDeck(QString);
	void fontChanged();

private:
	Ui::CollectionWindow ui_;
	CollectionTableModel collectionTableModel_;
	QScopedPointer<QAbstractItemDelegate> itemDelegate_;
	FilterNode::Ptr rootFilterNode_;

	virtual void closeEvent(QCloseEvent* event);
	virtual bool event(QEvent* event);
	int currentDataRowIndex() const;
	QVector<int> currentDataRowIndices() const;
	void updateStatusBar();

public slots:
	void addToCollection(const QVector<int>&);
	void addToCollection(const QVector<QPair<int, int>>&);
	void removeFromCollection(const QVector<int>&);

private slots:
	void currentRowChanged(QModelIndex, QModelIndex);
	void dataChanged(QModelIndex, QModelIndex);
	void actionAdvancedFilter();
	void actionEnableFilter(bool enable);
	void actionAddToCollection();
	void actionRemoveFromCollection();
	void actionAddToDeck();
	void actionRemoveFromDeck();
	void actionFetchOnlineData();
	void hideColumnsContextMenuRequested(const QPoint& pos);
	void rowContextMenuRequested(const QPoint& pos);
	void updateUsedCount();
	void handleGlobalFilterChanged();
};
