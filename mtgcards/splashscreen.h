#pragma once

#include <QWidget>
#include <QSplashScreen>

class SplashScreen
{
public:
	SplashScreen();
	~SplashScreen();

	void showMessage(const QString& message);
	void finish(QWidget* mainWin);

private:
	QSplashScreen* splashScreen_;
};
