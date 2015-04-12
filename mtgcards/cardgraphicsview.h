#pragma once

#include <QGraphicsView>

class CardGraphicsView : public QGraphicsView
{
public:
	CardGraphicsView(QWidget* parent);
	~CardGraphicsView();

	virtual void resizeEvent(QResizeEvent* event);
};
