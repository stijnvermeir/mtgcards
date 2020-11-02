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

public slots:
	void setMessage(const QString& message);
	void setSearch(const QString& search);
private:
	Ui::StatusBar *ui;
};

