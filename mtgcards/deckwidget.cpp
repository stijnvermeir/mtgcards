#include "deckwidget.h"

DeckWidget::DeckWidget(QWidget *parent)
	: QWidget(parent)
	, ui_()
{
	ui_.setupUi(this);
}

DeckWidget::~DeckWidget()
{
}
