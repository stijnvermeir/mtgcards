#include "statusbar.h"
#include "ui_statusbar.h"

StatusBar::StatusBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatusBar)
{
	ui->setupUi(this);

	ui->viewComboBox->addItem("List");
	ui->viewComboBox->addItem("Images");
	connect(ui->viewComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(viewChanged(int)));

	ui->imageSizeSlider->setVisible(false);
	connect(ui->imageSizeSlider, SIGNAL(valueChanged(int)), this, SIGNAL(sliderValueChanged(int)));
}

StatusBar::~StatusBar()
{
	delete ui;
}

void StatusBar::setViewChangerEnabled(bool enable)
{
	ui->viewComboBox->setVisible(enable);
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
	return ui->viewComboBox->currentIndex();
}

void StatusBar::setViewIndex(int index)
{
	ui->viewComboBox->setCurrentIndex(index);
}

void StatusBar::setMessage(const QString& message)
{
	ui->statusLabel->setText(message);
}

void StatusBar::setSearch(const QString& search)
{
	ui->searchLabel->setText(search);
}
