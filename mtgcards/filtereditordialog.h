#pragma once

#include "ui_filtereditordialog.h"
#include "filtermodel.h"

#include <QDialog>
#include <QScopedPointer>

class QAbstractItemDelegate;

class FilterEditorDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FilterEditorDialog(QWidget *parent = 0);
	~FilterEditorDialog();

	void setFilterRootNode(const FilterNode::Ptr& rootNode);
	const FilterNode::Ptr& getFilterRootNode() const;

private:
	Ui::FilterEditorDialog ui_;
	FilterModel model_;
	QScopedPointer<QAbstractItemDelegate> itemDelegate_;

private slots:
	void newBtnClicked();
	void openBtnClicked();
	void saveBtnClicked();
	void addGroupBtnClicked();
	void addFilterBtnClicked();
	void deleteNodeBtnClicked();
};
