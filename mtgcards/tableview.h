#pragma once

#include <QTableView>

class TableView : public QTableView
{
	Q_OBJECT

public:
	TableView(QWidget* parent = 0);
	virtual ~TableView();

	virtual void keyboardSearch(const QString& search);
};
