#include "statusbar.h"
#include "ui_statusbar.h"

StatusBar::StatusBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatusBar)
{
	ui->setupUi(this);
}

StatusBar::~StatusBar()
{
	delete ui;
}

void StatusBar::setMessage(const QString& message)
{
	ui->statusLabel->setText(message);
}

void StatusBar::setSearch(const QString& search)
{
	ui->searchLabel->setText(search);
}
