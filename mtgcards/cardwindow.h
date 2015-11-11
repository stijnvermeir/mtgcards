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
	explicit CardWindow(QWidget* parent = 0);
	~CardWindow();

signals:
	void windowClosed(bool);

private:
	Ui::CardWindow ui_;
	QGraphicsScene scene_;
	mtg::LayoutType layoutType_;
	QStringList imageFiles_;
	bool secondViewActive_;
	int picturePage_;

	void closeEvent(QCloseEvent* event);
	bool eventFilter(QObject* object, QEvent* event);
	void setCardPicture(const QString& imageFile, double rotation);

public slots:
	void changeCardPicture(int);

private slots:
	void switchPicture();
};
