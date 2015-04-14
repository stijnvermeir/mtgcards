#pragma once

#include <QStyledItemDelegate>

class RichTextDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	RichTextDelegate(QWidget* parent = 0)
		: QStyledItemDelegate(parent) {}

	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};
