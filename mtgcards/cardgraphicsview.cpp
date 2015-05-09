#include "cardgraphicsview.h"

#include <QResizeEvent>
#include <QMouseEvent>

CardGraphicsView::CardGraphicsView(QWidget* parent)
	: QGraphicsView(parent)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	installEventFilter(this);
}

CardGraphicsView::~CardGraphicsView()
{
}

void CardGraphicsView::resizeEvent(QResizeEvent* event)
{
	if (!items().empty())
	{
		fitInView(items().first(), Qt::KeepAspectRatio);
		centerOn(items().first());
	}
	event->accept();
}

void CardGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
	emit clicked();
	event->accept();
}

bool CardGraphicsView::eventFilter(QObject*, QEvent* event)
{
	if (event->type() == QEvent::Wheel)
	{
		return true;
	}
	return false;
}
