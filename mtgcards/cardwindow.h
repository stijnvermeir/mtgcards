#pragma once

#include "magiclayouttype.h"
#include "ui_cardwindow.h"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>

class CardWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit CardWindow(QWidget *parent = 0);
	~CardWindow();

signals:
	void windowClosed(bool);

private:
	Ui::CardWindow ui_;
	QGraphicsScene scene_;
	mtg::LayoutType layoutType_;
	QStringList imageFiles_;
	bool secondViewActive_;
	QTimer timer_;

	void closeEvent(QCloseEvent* event);
	void setCardPicture(const QString& imageFile, double rotation);

public slots:
	void changeCardPicture(mtg::LayoutType, QStringList);

private slots:
	void timerExpired();
};
