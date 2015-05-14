#include "filtereditordialog.h"

#include "settings.h"

#include <QStyledItemDelegate>
#include <QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QComboBox>

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
				if (index.column() == FilterModel::Column::Type)
				{
					QComboBox* typeCbx = new QComboBox(parent);
					for (const auto& type : FilterFunctionManager::instance().getRegisteredFunctions())
					{
						typeCbx->addItem(type);
					}
					if (filterNode->getFilter().function)
					{
						typeCbx->setCurrentText(filterNode->getFilter().function->getId());
					}
					return typeCbx;
				}
				else
				if (index.column() == FilterModel::Column::Field)
				{
					QComboBox* columnCbx = new QComboBox(parent);
					for (const auto& col : mtg::ColumnType::list())
					{
						columnCbx->addItem(col.getDisplayName(), QVariant::fromValue(col));
					}
					columnCbx->setCurrentText(filterNode->getFilter().column.getDisplayName());
					return columnCbx;
				}
				else
				if (index.column() == FilterModel::Column::Filter)
				{
					if (filterNode->getFilter().function)
					{
						return filterNode->getFilter().function->createEditor(parent);
					}
				}
			}
			else
			{
				if (index.column() == FilterModel::Column::Type)
				{
					QComboBox* nodeTypeCbx = new QComboBox(parent);
					nodeTypeCbx->addItem(static_cast<QString>(FilterNode::Type(FilterNode::Type::AND)));
					nodeTypeCbx->addItem(static_cast<QString>(FilterNode::Type(FilterNode::Type::OR)));
					nodeTypeCbx->setCurrentText(filterNode->getType());
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
					if (index.column() == FilterModel::Column::Type)
					{
						QComboBox* typeCbx = static_cast<QComboBox*>(editor);
						if (!filterNode->getFilter().function || filterNode->getFilter().function->getId() != typeCbx->currentText())
						{
							filterNode->getFilter().function = FilterFunctionManager::instance().createFromId(typeCbx->currentText());
						}
					}
					else
					if (index.column() == FilterModel::Column::Field)
					{
						QComboBox* columnCbx = static_cast<QComboBox*>(editor);
						filterNode->getFilter().column = qvariant_cast<mtg::ColumnType>(columnCbx->currentData());
					}
					else
					if (index.column() == FilterModel::Column::Filter)
					{
						if (filterNode->getFilter().function)
						{
							filterNode->getFilter().function->updateFromEditor(editor);
						}
					}
				}
				else
				{
					if (index.column() == FilterModel::Column::Type)
					{
						QComboBox* nodeTypeCbx = static_cast<QComboBox*>(editor);
						filterNode->setType(nodeTypeCbx->currentText());
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
	, itemDelegate_(new FilterItemDelegate())
{
	ui_.setupUi(this);
	ui_.treeView->setModel(&model_);
	ui_.treeView->setItemDelegate(itemDelegate_.data());

	connect(ui_.newBtn, SIGNAL(released()), this, SLOT(newBtnClicked()));
	connect(ui_.openBtn, SIGNAL(released()), this, SLOT(openBtnClicked()));
	connect(ui_.saveBtn, SIGNAL(released()), this, SLOT(saveBtnClicked()));
	connect(ui_.addGroupBtn, SIGNAL(released()), this, SLOT(addGroupBtnClicked()));
	connect(ui_.addFilterBtn, SIGNAL(released()), this, SLOT(addFilterBtnClicked()));
	connect(ui_.deleteNodeBtn, SIGNAL(released()), this, SLOT(deleteNodeBtnClicked()));

	ui_.newBtn->setShortcut(Settings::instance().getShortcuts()[ShortcutType::NewFile]);
	ui_.openBtn->setShortcut(Settings::instance().getShortcuts()[ShortcutType::OpenFile]);
	ui_.saveBtn->setShortcut(Settings::instance().getShortcuts()[ShortcutType::SaveFile]);
	ui_.addGroupBtn->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AddFilterGroup]);
	ui_.addFilterBtn->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AddFilterNode]);
	ui_.deleteNodeBtn->setShortcut(Settings::instance().getShortcuts()[ShortcutType::RemoveFilterNode]);
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
	auto filename = QFileDialog::getOpenFileName(this, "Open Filter Tree file", Settings::instance().getFiltersDir(), "Filters (*.filter)");
	if (!filename.isNull())
	{
		model_.setFilterRootNode(FilterNode::createFromFile(filename));
		ui_.treeView->expandAll();
	}
}

void FilterEditorDialog::saveBtnClicked()
{
	auto filename = QFileDialog::getSaveFileName(this, "Save Filter Tree file", Settings::instance().getFiltersDir(), "Filters (*.filter)");
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
	auto firstFilterFuncId = FilterFunctionManager::instance().getRegisteredFunctions().front();
	child->getFilter().function = FilterFunctionManager::instance().createFromId(firstFilterFuncId);

	auto currentIndex = ui_.treeView->currentIndex();
	model_.addNode(child, currentIndex);
	ui_.treeView->expandAll();
	ui_.treeView->setCurrentIndex(currentIndex);
}

void FilterEditorDialog::deleteNodeBtnClicked()
{
	model_.deleteNode(ui_.treeView->currentIndex());
	ui_.treeView->expandAll();
	// TODO: set some node that makes sense as current
}
