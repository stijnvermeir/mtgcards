#include "cardwindow.h"

#include <QSettings>
#include <QCloseEvent>
#include <QGraphicsPixmapItem>
#include <QDebug>

namespace {

const int PICTURE_SWITCH_INTERVAL = 5000; // ms

} // namespace

CardWindow::CardWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, scene_()
	, layoutType_(layout_type_t::Normal)
	, imageFiles_()
	, secondViewActive_(false)
	, timer_()
{
	ui_.setupUi(this);
	ui_.cardView_->setScene(&scene_);

	connect(&timer_, SIGNAL(timeout()), this, SLOT(timerExpired()));

}

CardWindow::~CardWindow()
{
}

void CardWindow::closeEvent(QCloseEvent* event)
{
	emit windowClosed(false);
	event->accept();
}

void CardWindow::setCardPicture(const QString& imageFile, double rotation)
{
	scene_.clear();
	auto item = new QGraphicsPixmapItem(QPixmap(imageFile));
	item->setTransformationMode(Qt::SmoothTransformation);
	item->setRotation(rotation);
	scene_.addItem(item);
	ui_.cardView_->fitInView(item, Qt::KeepAspectRatio);
}

void CardWindow::changeCardPicture(layout_type_t layout, QStringList imageFiles)
{
	imageFiles_ = imageFiles;
	layoutType_ = layout;

	if (layoutType_ == layout_type_t::Split)
	{
		setCardPicture(imageFiles_.front(), 90);
	}
	else
	if (layoutType_ == layout_type_t::DoubleFaced)
	{
		secondViewActive_ = false;
		setCardPicture(imageFiles_.front(), 0);
		timer_.start(PICTURE_SWITCH_INTERVAL);
	}
	else
	if (layoutType_ == layout_type_t::Flip)
	{
		secondViewActive_ = false;
		setCardPicture(imageFiles_.front(), 0);
		timer_.start(PICTURE_SWITCH_INTERVAL);
	}
	else
	{
		setCardPicture(imageFiles_.front(), 0);
	}
}

void CardWindow::timerExpired()
{
	if (layoutType_ == layout_type_t::DoubleFaced)
	{
		if (secondViewActive_)
		{
			setCardPicture(imageFiles_.front(), 0);
		}
		else
		{
			setCardPicture(imageFiles_.back(), 0);
		}
		secondViewActive_ = !secondViewActive_;
		timer_.start(PICTURE_SWITCH_INTERVAL);
	}
	else
	if (layoutType_ == layout_type_t::Flip)
	{
		if (secondViewActive_)
		{
			setCardPicture(imageFiles_.front(), 0);
		}
		else
		{
			setCardPicture(imageFiles_.front(), 180);
		}
		secondViewActive_ = !secondViewActive_;
		timer_.start(PICTURE_SWITCH_INTERVAL);
	}
}
