#include "statusbar.h"
#include "ui_statusbar.h"

StatusBar::StatusBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatusBar)
{
	ui->setupUi(this);

	connect(ui->listViewBtn, SIGNAL(clicked(bool)), this, SLOT(listViewBtnClicked(bool)));
	connect(ui->gridViewBtn, SIGNAL(clicked(bool)), this, SLOT(gridViewBtnClicked(bool)));

	ui->imageSizeSlider->setVisible(false);
	connect(ui->imageSizeSlider, SIGNAL(valueChanged(int)), this, SIGNAL(sliderValueChanged(int)));
}

StatusBar::~StatusBar()
{
	delete ui;
}

void StatusBar::setViewChangerEnabled(bool enable)
{
	ui->listViewBtn->setVisible(enable);
	ui->gridViewBtn->setVisible(enable);
}

void StatusBar::setSearchEnabled(bool enable)
{
	ui->searchPrefixLabel->setVisible(enable);
	ui->searchLabel->setVisible(enable);
}

void StatusBar::setSliderEnabled(bool enable)
{
	ui->imageSizeSlider->setVisible(enable);
}

int StatusBar::getSliderValue() const
{
	return ui->imageSizeSlider->value();
}

void StatusBar::setSliderValue(int value)
{
	ui->imageSizeSlider->setValue(value);
}

int StatusBar::getViewIndex() const
{
	if (ui->listViewBtn->isChecked())
	{
		return 0;
	}
	return 1;
}

void StatusBar::setViewIndex(int index)
{
	if (index == 0)
	{
		ui->listViewBtn->setChecked(true);
		ui->gridViewBtn->setChecked(false);
		emit viewChanged(0);
	}
	else
	{
		ui->listViewBtn->setChecked(false);
		ui->gridViewBtn->setChecked(true);
		emit viewChanged(1);
	}
}

void StatusBar::listViewBtnClicked(bool checked)
{
	setViewIndex(checked ? 0 : 1);
}

void StatusBar::gridViewBtnClicked(bool checked)
{
	setViewIndex(checked ? 1 : 0);
}

void StatusBar::setMessage(const QString& message)
{
	ui->statusLabel->setText(message);
}

void StatusBar::setSearch(const QString& search)
{
	ui->searchLabel->setText(search);
}
