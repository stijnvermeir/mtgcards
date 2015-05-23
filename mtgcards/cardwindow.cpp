#include "cardwindow.h"

#include "magiccarddata.h"

#include <QCloseEvent>
#include <QGraphicsPixmapItem>
#include <QDebug>

CardWindow::CardWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui_()
	, scene_()
	, layoutType_(mtg::LayoutType::Normal)
	, imageFiles_()
	, secondViewActive_(false)
{
	setWindowFlags(Qt::NoDropShadowWindowHint);
	ui_.setupUi(this);
	ui_.cardView_->setScene(&scene_);
	connect(ui_.cardView_, SIGNAL(clicked()), this, SLOT(switchPicture()));
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
	item->setTransformOriginPoint(item->boundingRect().center());
	item->setRotation(rotation);
	scene_.addItem(item);
	ui_.cardView_->fitInView(item, Qt::KeepAspectRatio);
	ui_.cardView_->centerOn(item);
}

void CardWindow::changeCardPicture(int row)
{
	auto rv = mtg::CardData::instance().getPictureFilenames(row);
	imageFiles_ = rv.second;
	layoutType_ = rv.first;
	secondViewActive_ = false;
	setCardPicture(imageFiles_.front(), 0);
}

void CardWindow::switchPicture()
{
	if (secondViewActive_)
	{
		setCardPicture(imageFiles_.front(), 0);
	}
	else
	{
		if (layoutType_ == mtg::LayoutType::Split)
		{
			setCardPicture(imageFiles_.front(), 90);
		}
		else
		if (layoutType_ == mtg::LayoutType::DoubleFaced)
		{
			setCardPicture(imageFiles_.back(), 0);
		}
		else
		if (layoutType_ == mtg::LayoutType::Flip)
		{
			setCardPicture(imageFiles_.front(), 180);
		}
	}
	secondViewActive_ = !secondViewActive_;
}
