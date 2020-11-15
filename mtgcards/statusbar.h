#pragma once

#include <QWidget>

namespace Ui {
class StatusBar;
}

class StatusBar : public QWidget
{
	Q_OBJECT

public:
	explicit StatusBar(QWidget *parent = nullptr);
	~StatusBar();

	void setViewChangerEnabled(bool enable);
	void setSearchEnabled(bool enable);
	void setSliderEnabled(bool enable);

	int getSliderValue() const;
	void setSliderValue(int value);

	int getViewIndex() const;
	void setViewIndex(int index);

signals:
	void viewChanged(int);
	void sliderValueChanged(int);

public slots:
	void setMessage(const QString& message);
	void setSearch(const QString& search);
private:
	Ui::StatusBar *ui;
};

