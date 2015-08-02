#include "treemodel.h"

void setupModelData(TreeItem* parent) {
	parent->insertChildren("sdsgf");
	parent->insertChildren("asdd")->insertChildren("qwedf");
	auto item = parent->insertChildren("ffxxgh");
	item->insertChildren("777");
	item->insertChildren("5555");
	parent->insertChildren("vbnmm")->insertChildren("1234");
	parent->insertChildren("vcxbn")->insertChildren("xccfg");
}

TreeModel::TreeModel(QObject* parent)
    : QAbstractItemModel(parent), 
	root_(new TreeItem()) {
    setupModelData(root_);
}

TreeModel::~TreeModel() {
    delete root_;
}

int TreeModel::columnCount(const QModelIndex&) const {
	return 1;
}

QVariant TreeModel::data(const QModelIndex& index, int role) const {
	auto item = ::item(index);
	if (!item) {
		qWarning() << "item is null";
		return{};
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole) {
		return item->data();
	}

	return {};
}

bool TreeModel::setData(
	const QModelIndex& index, 
	const QVariant& value, 
	int role
) {
	if (role == Qt::EditRole) {
		auto item = ::item(index);
		if (!item) {
			qWarning() << "item is null";
			return false;
		}

		auto data = value.toString().trimmed();

		if (!data.isEmpty() && item->setData(data)) {
			emit dataChanged(index, index);
			return true;
		}
	}

	return false;
}


QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
	if (!hasIndex(row, column, parent)) {
		qWarning() << "invalid arguments: hasIndex return false";
		return {};
	}

	auto parentItem = item(parent, root_);
	if (!parentItem) {
		qWarning() << "parent item is null";
		return {};
	}

	
	auto childItem = parentItem->child(row);
	if (!childItem) {
		qWarning() << "child item is null";
		return{};
	}

	return createIndex(row, column, childItem);
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
	auto childItem = item(index);
	if (!childItem) {
		qWarning() << "item is null";
		return {};
	}

	auto parentItem = childItem->parent();
	if (parentItem != root_) {
		return createIndex(parentItem->row(), 0, parentItem);
	}
	
	return {};
}

int TreeModel::rowCount(const QModelIndex& parent) const 
{
	auto parentItem = item(parent, root_);

	if (!parentItem) {
		qWarning() << "item is null";
		return 0;
	}

	return parentItem->childCount();
}

bool TreeModel::insertRows(int pos, int rows, const QModelIndex &parent) {
	if (pos < 0 || rows != 1) {
		qWarning() << "invalid arguments";
		return false;
	}

	auto parentItem = ::item(parent, root_);
	if (!parentItem) {
		qWarning() << "item is null";
		return false;
	}

	if (pos > parentItem->childCount()) {
		qWarning() << "invalid argument (position)";
		return false;
	}

	if (parentItem->isEmptyChild()) {
		qWarning() << "item already insert";
		return false;
	}

	beginInsertRows(parent, pos, pos + rows - 1);
	auto ok = parentItem->insertChildren({}, pos) != nullptr;
	endInsertRows();
	return ok;
}

bool TreeModel::removeRows(int pos, int count, const QModelIndex &parent) {
	if (pos < 0 || count <= 0) {
		qWarning() << "invalid arguments";
		return false;
	}

	auto parentItem = ::item(parent, root_);
	if (parentItem == nullptr) {
		qWarning() << "item is null";
		return false;
	}

	auto childCount = parentItem->childCount();
	if (childCount <= 0 || pos >= childCount || count > (childCount - pos)) {
		qWarning() << "invalid arguments";
		return false;
	}

	beginRemoveRows(parent, pos, pos + count - 1);
	parentItem->removeChildren(pos, count);
	endRemoveRows();

	return true;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = Qt::ItemIsDragEnabled;

	if (index.isValid()) {
		flags |= Qt::ItemIsEditable;
		flags |= QAbstractItemModel::flags(index);
	}

	return flags;
}

const QString& treeMimeType() {
	static const QString type( QStringLiteral("text/xml") );
	return type;
}


QStringList TreeModel::mimeTypes() const {
	return { treeMimeType() };
}

Qt::DropActions TreeModel::supportedDropActions() const {
	return Qt::CopyAction | Qt::MoveAction;
}

QMimeData *TreeModel::mimeData(const QModelIndexList &indexes) const
{
	return nullptr;
}

bool TreeModel::dropMimeData(
	const QMimeData * /*data*/, 
	Qt::DropAction /*action*/,
	int /*row*/, int /*column*/, 
	const QModelIndex &/*parent*/
) {
	/*if (!data->hasFormat("image/x-puzzle-piece"))
		return false;

	if (action == Qt::IgnoreAction)
		return true;

	if (column > 0)
		return false;

	int endRow;

	if (!parent.isValid()) {
		if (row < 0)
			endRow = pixmaps.size();
		else
			endRow = qMin(row, pixmaps.size());
	}
	else {
		endRow = parent.row();
	}

	QByteArray encodedData = data->data("image/x-puzzle-piece");
	QDataStream stream(&encodedData, QIODevice::ReadOnly);

	while (!stream.atEnd()) {
		QPixmap pixmap;
		QPoint location;
		stream >> pixmap >> location;

		beginInsertRows(QModelIndex(), endRow, endRow);
		pixmaps.insert(endRow, pixmap);
		locations.insert(endRow, location);
		endInsertRows();

		++endRow;
	}*/

	return true;
}
