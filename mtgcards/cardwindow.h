#pragma once

#include "enum.h"
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
	layout_type_t layoutType_;
	QStringList imageFiles_;
	bool secondViewActive_;
	QTimer timer_;

	void closeEvent(QCloseEvent* event);
	void setCardPicture(const QString& imageFile, double rotation);

public slots:
	void changeCardPicture(layout_type_t, QStringList);

private slots:
	void timerExpired();
};
