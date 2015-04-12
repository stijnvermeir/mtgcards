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

	QSettings settings;
	resize(settings.value("cardwindow/size", QSize(480, 680)).toSize());
	move(settings.value("cardwindow/pos", QPoint(0, 77)).toPoint());

	connect(&timer_, SIGNAL(timeout()), this, SLOT(timerExpired()));

}

CardWindow::~CardWindow()
{
}

void CardWindow::closeEvent(QCloseEvent* event)
{
	qDebug() << "Closing Card Window";
	QSettings settings;
	settings.setValue("cardwindow/size", size());
	settings.setValue("cardwindow/pos", pos());
	emit windowClosed(false);
	if (event)
	{
		event->accept();
	}
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
	qDebug() << "Changing card picture";
	qDebug() << "Layout: " << static_cast<int>(layout);
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
