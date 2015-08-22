#pragma once

#include "ui_deckstatisticsdialog.h"

#include <QDialog>

class Deck;

class DeckStatisticsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit DeckStatisticsDialog(const Deck& deck, QWidget* parent = 0);
	~DeckStatisticsDialog();

private:
	Ui::DeckStatisticsDialog ui_;
};

