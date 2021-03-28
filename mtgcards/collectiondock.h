#pragma once

#include "collectiontablemodel.h"
#include "imagetablemodel.h"
#include "filter.h"
#include "commonactions.h"
#include "ui_mainwindow.h"

#include <QObject>
#include <QVector>
#include <QScopedPointer>

class QAbstractItemDelegate;

class CollectionDock : public QObject
{
	Q_OBJECT

public:
	explicit CollectionDock(Ui::MainWindow& ui, QWidget* parent = 0);
	virtual ~CollectionDock();

	void reload();
	void updateOptions();
	void loadSettings();
	void saveSettings();

signals:
	void selectedCardChanged(int);
	void addToDeck(QVector<int>);
	void removeFromDeck(QVector<int>);
	void requestOpenDeck(QString);
	void fontChanged();

private:
	Ui::MainWindow& ui_;
	CollectionTableModel collectionTableModel_;
	ImageTableModel imageTableModel_;
	QScopedPointer<QAbstractItemDelegate> itemDelegate_;
	FilterNode::Ptr rootFilterNode_;
	CommonActions commonActions_;

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
	void actionDownloadCardArt();
	void actionFetchOnlineData();
	void hideColumnsContextMenuRequested(const QPoint& pos);
	void rowContextMenuRequested(const QPoint& pos);
	void updateUsedCount();
	void handleGlobalFilterChanged();
	void statusBarViewChanged(int index);
	void imageCurrentChanged(const QModelIndex&, const QModelIndex&);
};
