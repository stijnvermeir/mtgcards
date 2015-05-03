#include "filtereditordialog.h"

#include <QStyledItemDelegate>
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
			if (filterNode->getType() == FilterNode::Type::LEAF && filterNode->getFilter().function)
			{
				QWidget* editor = new QWidget(parent);
				auto hl = new QHBoxLayout(editor);
				hl->setContentsMargins(0, 0, 0, 0);
				auto columnCbx = new QComboBox(editor);
				for (const auto& col : mtg::ColumnType::list())
				{
					columnCbx->addItem(col);
				}
				columnCbx->setCurrentText(filterNode->getFilter().column);
				hl->addWidget(columnCbx);

				if (filterNode->getFilter().function->getType() == FilterFunctionType::Regex)
				{
					auto regexTxt = new QLineEdit(editor);
					regexTxt->setText(static_cast<RegexFilterFunction*>(filterNode->getFilter().function.get())->getRegex().pattern());
					hl->addWidget(regexTxt);
				}


				editor->setAutoFillBackground(true);
				editor->setLayout(hl);
				editor->resize(200, 50);

				return editor;
			}
		}
		return nullptr;
	}

	virtual QSize sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
	{
		return QSize(300, 40);
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
