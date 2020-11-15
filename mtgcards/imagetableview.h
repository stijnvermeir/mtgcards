#pragma once

#include "imagetablemodel.h"

#include <QTableView>
#include <QObject>

class ImageTableView : public QTableView
{
	Q_OBJECT
public:
	ImageTableView(QWidget* parent = 0);

	void setImageSize(const QSize& size);
	void setImageTableModel(ImageTableModel* model);

public slots:
	void changeImageScale(int promille);

protected:
	void resizeEvent(QResizeEvent* event) override;
private:
	ImageTableModel* model_;
};

