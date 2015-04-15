#include "deckwindow.h"

#include <QSettings>
#include <QCloseEvent>
#include <QDebug>

DeckWindow::DeckWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
{
	ui_.setupUi(this);
}

DeckWindow::~DeckWindow()
{
}

void DeckWindow::closeEvent(QCloseEvent* event)
{
	emit windowClosed(false);
	event->accept();
}
