#include "deckwindow.h"

#include <QSettings>
#include <QCloseEvent>
#include <QDebug>

DeckWindow::DeckWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
{
	ui_.setupUi(this);

	QSettings settings;
	resize(settings.value("deckwindow/size", QSize(1914, 360)).toSize());
	move(settings.value("deckwindow/pos", QPoint(0, 786)).toPoint());
}

DeckWindow::~DeckWindow()
{
}

void DeckWindow::closeEvent(QCloseEvent* event)
{
	qDebug() << "Closing Deck Window";
	QSettings settings;
	settings.setValue("deckwindow/size", size());
	settings.setValue("deckwindow/pos", pos());
	emit windowClosed(false);
	if (event)
	{
		event->accept();
	}
}
