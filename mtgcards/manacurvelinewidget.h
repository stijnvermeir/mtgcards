#pragma once

#include "ui_manacurvelinewidget.h"

#include <QWidget>

class ManaCurveLineWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ManaCurveLineWidget(QWidget* parent = 0);
	~ManaCurveLineWidget();

	void setCmc(const QString& cmc);
	void setCount(int count);
	void setMax(int max);
private:
	Ui::ManaCurveLineWidget ui_;
};
