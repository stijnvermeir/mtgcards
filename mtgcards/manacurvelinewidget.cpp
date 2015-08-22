#include "manacurvelinewidget.h"
#include "manacost.h"

namespace
{
#ifndef Q_OS_OSX
const int MAGIC_SYMBOL_FONT_SIZE = 16;
#else
const int MAGIC_SYMBOL_FONT_SIZE = 20;
#endif
}

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
	ui_.cmcTxt->setText(ManaCost::replaceTagsWithSymbols(cmc, MAGIC_SYMBOL_FONT_SIZE));
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
