#include "treewidget.h"

QSize buttonsIconSize() {
	enum { WIDTH = 16 };
	enum { HEIGHT = 16 };

	return QSize(WIDTH, HEIGHT);
}

TreeWidget::TreeWidget(QWidget *parent)
	: QTreeView(parent),
	sourceModel_(nullptr),
	model_(nullptr),
	itemDelegate_(nullptr)
{
	sourceModel_ = new TreeModel(this);
	model_ = new SortFilterProxyModel(sourceModel_);
	model_->setSourceModel(sourceModel_);
	model_->setFilterFixedString({});
	model_->setFilterCaseSensitivity(Qt::CaseInsensitive);
	model_->setFilterKeyColumn(0);
	setModel(model_);

	itemDelegate_ = new ButtonsDelegate(buttonsIconSize(), this);
	QObject::connect(itemDelegate_, &ButtonsDelegate::removeClicked,
		[this](const QModelIndex& index) { this->removeRow(index.row(), index.parent()); }
	);
	QObject::connect(itemDelegate_, &ButtonsDelegate::addClicked,
		[this](const QModelIndex& index) { insertRow(index.row(), index.parent()); }
	);
	QObject::connect(itemDelegate_, &ButtonsDelegate::addChildClicked,
		[this](const QModelIndex& index) { insertRow(0, index); }
	);
	setItemDelegate(itemDelegate_);
}

TreeWidget::~TreeWidget() {}

void TreeWidget::closeEditor() {
	setEnabled(!isEnabled());
	setEnabled(!isEnabled());
}

void TreeWidget::insertRow(int row, const QModelIndex& parent) {
	qDebug() << "insertRow";
	closeEditor();

	QModelIndex editIndex = model_->mapFromSource(
		sourceModel_->index(QString(), model_->mapToSource(parent))
	);

	if (!editIndex.isValid()) {
		model_->insertRow(row, parent);
		editIndex = model_->index(row, 0, parent);
	}

	if (parent.isValid()) {
		expand(parent);
	}

	edit(editIndex);
}

void TreeWidget::removeRow(int row, const QModelIndex& parent) {
	if (!parent.isValid() && sourceModel_->rowCount() <= 1) {
		qDebug() << "remove the last item is not allowed";
		return;
	}

	qDebug() << "remove row";
	model_->removeRow(row, parent);
}

void TreeWidget::search(const QString& searchText) {
	model_->setFilterFixedString(searchText);
	if (!searchText.isEmpty()) {
		for (auto& expandIndex : model_->indexesExpand()) {
			expand(expandIndex);
		}
	}
}

QByteArray TreeWidget::serialize() const {
	return sourceModel_->serialize();
}

void TreeWidget::deserialize(const QByteArray& data) {
	sourceModel_->deserialize(data);
	if (sourceModel_->rowCount() <= 0) {
		sourceModel_->insertRow(0);
	}
}
