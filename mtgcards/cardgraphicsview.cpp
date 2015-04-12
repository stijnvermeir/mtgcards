#include "cardgraphicsview.h"

#include <QResizeEvent>

CardGraphicsView::CardGraphicsView(QWidget* parent)
	: QGraphicsView(parent)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

CardGraphicsView::~CardGraphicsView()
{
}

void CardGraphicsView::resizeEvent(QResizeEvent* event)
{
	if (!items().empty())
	{
		fitInView(items().first(), Qt::KeepAspectRatio);
	}
	event->accept();
}
