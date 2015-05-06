#pragma once

#include <QGraphicsView>

class CardGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
	CardGraphicsView(QWidget* parent);
	~CardGraphicsView();

	virtual void resizeEvent(QResizeEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual bool eventFilter(QObject* object, QEvent* event);

signals:
	void clicked();
};
