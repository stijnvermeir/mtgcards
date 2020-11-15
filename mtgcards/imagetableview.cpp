#include "imagetableview.h"

#include <QHeaderView>
#include <QResizeEvent>
#include <QDebug>

ImageTableView::ImageTableView(QWidget* parent)
    : QTableView(parent)
    , model_(nullptr)
{
	horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	horizontalHeader()->setDefaultSectionSize(240);
	horizontalHeader()->setVisible(false);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader()->setDefaultSectionSize(340);
	verticalHeader()->setVisible(false);
}

void ImageTableView::setImageSize(const QSize& imageSize)
{
	horizontalHeader()->setDefaultSectionSize(imageSize.width());
	verticalHeader()->setDefaultSectionSize(imageSize.height());
	if (model_)
	{
		model_->setNumColumns(size().width() / imageSize.width());
		model_->setImageSize(imageSize);
	}
}

void ImageTableView::setImageTableModel(ImageTableModel* model)
{
	model_ = model;
	setModel(model);
}

void ImageTableView::changeImageScale(int promille)
{
	setImageSize(QSize(480 * promille / 1000, 680 * promille / 1000));
}

void ImageTableView::resizeEvent(QResizeEvent* event)
{
	if (model_)
	{
		model_->setNumColumns(event->size().width() / horizontalHeader()->defaultSectionSize());
	}
	QTableView::resizeEvent(event);
}
