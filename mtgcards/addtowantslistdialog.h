#pragma once

#include "util.h"

#include <QDialog>
#include <QVector>

namespace Ui {
class AddToWantslistDialog;
}

class AddToWantslistDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AddToWantslistDialog(const QVector<int>& selection, QWidget* parent = 0);
	~AddToWantslistDialog();

private:
	Ui::AddToWantslistDialog* ui_;
	QVector<int> selection_;

	void loadWantslists(const QVector<mkm::Wantslist>& wantslists);

private slots:
	void addWantslistClicked();
	void addWants();
};
