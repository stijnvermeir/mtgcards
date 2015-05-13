#include "tableview.h"

#include <QDebug>

TableView::TableView(QWidget* parent)
	: QTableView(parent)
{
}

TableView::~TableView()
{
}

void TableView::keyboardSearch(const QString& search)
{
//	qDebug() << search;
//	qDebug() << itemDelegate()->metaObject()->className();

	QTableView::keyboardSearch(search);
}
