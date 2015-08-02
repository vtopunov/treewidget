#include "treewidget.h"

QSize buttonsIconSize() {
	enum { WIDTH = 16 };
	enum { HEIGHT = 16 };

	return QSize(WIDTH, HEIGHT);
}

QModelIndex findSiblingItem(const QModelIndex& index, const QString& text) {
	auto item = ::item(index);
	if (!item) {
		qWarning() << "item is null";
		return{};
	}
	auto emptyItemIndex = item->findSibling(text);
	bool isEmptyItem = emptyItemIndex >= 0;
	return (isEmptyItem) ? index.sibling(emptyItemIndex, 0) : QModelIndex();
}

QModelIndex findChildItem(QSortFilterProxyModel* model, const QModelIndex& parent, const QString& text) {
	auto index = model->index(0, 0, parent);
	if (index.isValid()) {
		index = findSiblingItem(model->mapToSource(index), text);
		if (index.isValid()) {
			return model->mapFromSource(index);
		}
	}
	return{};
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
	setDisabled(true);
	setDisabled(false);
}

void TreeWidget::insertRow(int row, const QModelIndex& parent) {
	qDebug() << "insertRow";
	closeEditor();

	auto editIndex = findChildItem(model_, parent, QString());
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
	for (auto& expandIndex : model_->indexesExpand()) {
		expand(expandIndex);
	}
}
