#pragma once

#include <QStyledItemDelegate>

class ManaCostDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	ManaCostDelegate(QWidget* parent = 0);

	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};
