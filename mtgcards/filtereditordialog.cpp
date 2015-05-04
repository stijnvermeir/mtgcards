#include "filtereditordialog.h"

#include <QStyledItemDelegate>
#include <QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>

namespace {

class FilterItemDelegate : public QStyledItemDelegate
{
public:
	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
	{
		FilterNode* filterNode = reinterpret_cast<FilterNode*>(index.internalPointer());
		if (filterNode)
		{
			if (filterNode->getType() == FilterNode::Type::LEAF)
			{
				if (index.column() == 1)
				{
					QComboBox* columnCbx = new QComboBox(parent);
					for (const auto& col : mtg::ColumnType::list())
					{
						columnCbx->addItem(col);
					}
					columnCbx->setCurrentText(filterNode->getFilter().column);
					return columnCbx;
				}
				else
				if (index.column() == 2)
				{
					if (filterNode->getFilter().function && filterNode->getFilter().function->getType() == FilterFunctionType::Regex)
					{
						QLineEdit* regexTxt = new QLineEdit(parent);
						regexTxt->setText(static_cast<RegexFilterFunction*>(filterNode->getFilter().function.get())->getRegex().pattern());
						return regexTxt;
					}
				}
			}
			else
			{
				if (index.column() == 0)
				{
					QComboBox* nodeTypeCbx = new QComboBox(parent);
					nodeTypeCbx->addItem("AND");
					nodeTypeCbx->addItem("OR");
					nodeTypeCbx->setCurrentText(filterNode->getType() == FilterNode::Type::AND ? "AND" : "OR");
					return nodeTypeCbx;
				}
			}
		}
		return nullptr;
	}

	virtual void setEditorData(QWidget*, const QModelIndex&) const {}

	virtual void setModelData(QWidget* editor, QAbstractItemModel*, const QModelIndex& index) const
	{
		if (editor)
		{
			FilterNode* filterNode = reinterpret_cast<FilterNode*>(index.internalPointer());
			if (filterNode)
			{
				if (filterNode->getType() == FilterNode::Type::LEAF)
				{
					if (index.column() == 1)
					{
						QComboBox* columnCbx = static_cast<QComboBox*>(editor);
						filterNode->getFilter().column = mtg::ColumnType(columnCbx->currentText());
					}
					else
					if (index.column() == 2)
					{
						if (filterNode->getFilter().function && filterNode->getFilter().function->getType() == FilterFunctionType::Regex)
						{
							QLineEdit* regexTxt = static_cast<QLineEdit*>(editor);
							static_cast<RegexFilterFunction*>(filterNode->getFilter().function.get())->setRegex(QRegularExpression(regexTxt->text()));
						}
					}
				}
				else
				{
					if (index.column() == 0)
					{
						QComboBox* nodeTypeCbx = static_cast<QComboBox*>(editor);
						filterNode->setType(nodeTypeCbx->currentText() == "AND" ? FilterNode::Type::AND : FilterNode::Type::OR);
					}
				}
			}
		}
	}

	virtual QSize sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
	{
		const int ROW_HEIGHT = 30;
		return QSize(0, ROW_HEIGHT);
	}
};

} // namespace

FilterEditorDialog::FilterEditorDialog(QWidget *parent)
	: QDialog(parent)
	, ui_()
	, model_()
{
	ui_.setupUi(this);
	ui_.treeView->setModel(&model_);
	ui_.treeView->setItemDelegate(new FilterItemDelegate());

	connect(ui_.newBtn, SIGNAL(released()), this, SLOT(newBtnClicked()));
	connect(ui_.openBtn, SIGNAL(released()), this, SLOT(openBtnClicked()));
	connect(ui_.saveBtn, SIGNAL(released()), this, SLOT(saveBtnClicked()));
	connect(ui_.addGroupBtn, SIGNAL(released()), this, SLOT(addGroupBtnClicked()));
	connect(ui_.addFilterBtn, SIGNAL(released()), this, SLOT(addFilterBtnClicked()));
	connect(ui_.deleteNodeBtn, SIGNAL(released()), this, SLOT(deleteNodeBtnClicked()));
}

FilterEditorDialog::~FilterEditorDialog()
{
}

void FilterEditorDialog::setFilterRootNode(const FilterNode::Ptr& rootNode)
{
	model_.setFilterRootNode(rootNode);
	ui_.treeView->expandAll();
}

const FilterNode::Ptr& FilterEditorDialog::getFilterRootNode() const
{
	return model_.getFilterRootNode();
}

void FilterEditorDialog::newBtnClicked()
{
	model_.setFilterRootNode(FilterNode::Ptr());
}

void FilterEditorDialog::openBtnClicked()
{
	auto filename = QFileDialog::getOpenFileName(this, "Open Filter Tree file", QDir::homePath(), "Filters (*.filter)");
	if (!filename.isNull())
	{
		model_.setFilterRootNode(FilterNode::createFromFile(filename));
		ui_.treeView->expandAll();
	}
}

void FilterEditorDialog::saveBtnClicked()
{
	auto filename = QFileDialog::getSaveFileName(this, "Save Filter Tree file", QDir::homePath(), "Filters (*.filter)");
	if (!filename.isNull())
	{
		model_.getFilterRootNode()->saveToFile(filename);
	}
}

void FilterEditorDialog::addGroupBtnClicked()
{
	FilterNode::Ptr groupNode = FilterNode::create();
	groupNode->setType(FilterNode::Type::AND);

	auto currentIndex = ui_.treeView->currentIndex();
	model_.addNode(groupNode, currentIndex);
	ui_.treeView->expandAll();
	ui_.treeView->setCurrentIndex(currentIndex);
}

void FilterEditorDialog::addFilterBtnClicked()
{
	FilterNode::Ptr child = FilterNode::create();
	child->getFilter().function = FilterFunctionFactory::createRegex("");

	auto currentIndex = ui_.treeView->currentIndex();
	model_.addNode(child, currentIndex);
	ui_.treeView->expandAll();
	ui_.treeView->setCurrentIndex(currentIndex);
}

void FilterEditorDialog::deleteNodeBtnClicked()
{
	model_.deleteNode(ui_.treeView->currentIndex());
	ui_.treeView->expandAll();
}
