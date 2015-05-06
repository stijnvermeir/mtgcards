#include "cardwindow.h"

#include "magiccarddata.h"

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
	, layoutType_(mtg::LayoutType::Normal)
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

void CardWindow::changeCardPicture(int row)
{
	auto rv = mtg::CardData::instance().getPictureFilenames(row);
	imageFiles_ = rv.second;
	layoutType_ = rv.first;

	if (layoutType_ == mtg::LayoutType::Split)
	{
		setCardPicture(imageFiles_.front(), 90);
	}
	else
	if (layoutType_ == mtg::LayoutType::DoubleFaced)
	{
		secondViewActive_ = false;
		setCardPicture(imageFiles_.front(), 0);
		timer_.start(PICTURE_SWITCH_INTERVAL);
	}
	else
	if (layoutType_ == mtg::LayoutType::Flip)
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
	if (layoutType_ == mtg::LayoutType::DoubleFaced)
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
	if (layoutType_ == mtg::LayoutType::Flip)
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
