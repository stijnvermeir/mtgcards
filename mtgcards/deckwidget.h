#pragma once

#include "ui_deckwidget.h"
#include <QWidget>

class DeckWidget : public QWidget
{
	Q_OBJECT

public:
	explicit DeckWidget(QWidget *parent = 0);
	~DeckWidget();

private:
	Ui::DeckWidget ui_;
};
