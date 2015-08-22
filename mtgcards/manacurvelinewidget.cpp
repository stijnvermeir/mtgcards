#include "manacurvelinewidget.h"
#include "manacost.h"

ManaCurveLineWidget::ManaCurveLineWidget(QWidget* parent)
	: QWidget(parent)
	, ui_()
{
	ui_.setupUi(this);
	ui_.verticalSlider->setEnabled(false);
}

ManaCurveLineWidget::~ManaCurveLineWidget()
{
}

void ManaCurveLineWidget::setCmc(const QString& cmc)
{
	ui_.cmcTxt->setText(ManaCost::replaceTagsWithSymbols(cmc, 20));
}

void ManaCurveLineWidget::setCount(int count)
{
	ui_.valueTxt->setText(QString::number(count));
	ui_.verticalSlider->setValue(count);
}

void ManaCurveLineWidget::setMax(int max)
{
	ui_.verticalSlider->setMaximum(max);
}
