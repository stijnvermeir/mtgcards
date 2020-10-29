#pragma once

#include "magiclayouttype.h"
#include "cardgraphicsview.h"
#include "ui_mainwindow.h"

#include <QObject>
#include <QGraphicsScene>
#include <QTimer>

class CardDock : public QObject
{
	Q_OBJECT

public:
	explicit CardDock(Ui::MainWindow& ui,  QWidget* parent = 0);
	~CardDock();

private:
	Ui::MainWindow& ui_;
	QGraphicsScene scene_;
	mtg::LayoutType layoutType_;
	QStringList imageFiles_;
	bool secondViewActive_;
	int picturePage_;

	bool eventFilter(QObject* object, QEvent* event);
	void setCardPicture(const QString& imageFile, double rotation);

public slots:
	void changeCardPicture(int);

private slots:
	void switchPicture();
};
