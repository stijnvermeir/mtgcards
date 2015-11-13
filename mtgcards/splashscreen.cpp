#include "splashscreen.h"

#include <QPixmap>
#include <QApplication>

SplashScreen::SplashScreen()
	: splashScreen_(nullptr)
{
	QPixmap pixmap(":/resources/icons/mtgcards.svg");
	splashScreen_ = new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint);
	splashScreen_->show();
}

SplashScreen::~SplashScreen()
{
	delete splashScreen_;
	splashScreen_ = nullptr;
}

void SplashScreen::showMessage(const QString& message)
{
	splashScreen_->showMessage(message, Qt::AlignHCenter | Qt::AlignBottom, Qt::black);
	QApplication::instance()->processEvents();
}

void SplashScreen::finish(QWidget* mainWin)
{
	splashScreen_->finish(mainWin);
}
