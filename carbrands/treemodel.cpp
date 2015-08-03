#include "treemodel.h"
#include <QtXml>

TreeModel::TreeModel(QObject* parent)
    : QAbstractItemModel(parent), 
	 root_(new TreeItem()) {}

TreeModel::~TreeModel() {
    delete root_;
}

// set/get data
////////////////////////////////////////////////////////////////////////////////
QVariant TreeModel::data(const QModelIndex& index, int role) const {
	auto item = this->item(index);

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
		auto item = this->item(index);
		auto data = value.toString().trimmed();
		if (!data.isEmpty() && item->setData(data)) {
			if (item != root_) {
				emit dataChanged(index, index);
			}
			return true;
		}
	}

	return false;
}

// index, parent, count
///////////////////////////////////////////////////////////////////////////////////////
QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
	if (!hasIndex(row, column, parent)) {
		qWarning() << "invalid arguments: hasIndex return false";
		return {};
	}

	auto item = this->item(parent)->child(row);
	if (!item) {
		qWarning() << "child item is null";
		return{};
	}

	return createIndex(row, column, item);
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
	auto childItem = item(index);
	if (childItem == root_) {
		qWarning() << "item is root";
		return {};
	}

	auto parentItem = childItem->parent();
	return (parentItem != root_)
		? createIndex(parentItem->row(), 0, parentItem)
		: QModelIndex();
}

int TreeModel::rowCount(const QModelIndex& parent) const {
	return item(parent)->childCount();
}

int TreeModel::columnCount(const QModelIndex&) const {
	return 1;
}

// insert, remove 
/////////////////////////////////////////////////////////////////////////////////////////
QModelIndex TreeModel::insert(const QString& data, int pos, const QModelIndex& parent) {
	auto parentItem = item(parent);

	if (pos < 0 || pos > parentItem->childCount()) {
		qWarning() << "invalid argument (position)";
		return{};
	}

	if (parentItem->index(data) != -1) {
		qWarning() << "data is not unique";
		return{};
	}

	beginInsertRows(parent, pos, pos);
	parentItem->insertChildren(data, pos);
	endInsertRows();

	return index(pos, 0, parent);
}

bool TreeModel::insertRows(int pos, int count, const QModelIndex &parent) {
	if (count != 1) {
		qWarning() << "model support insert only one row, because each item must be unique";
		return false;
	}

	return insert({}, pos, parent).isValid();
}

bool TreeModel::removeRows(int pos, int count, const QModelIndex &parent) {
	if (pos < 0 || count <= 0) {
		qWarning() << "invalid arguments";
		return false;
	}

	auto parentItem = item(parent);

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

// flags
///////////////////////////////////////////////////////////////////////////////////////
Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

	if (index.isValid()) {
		flags |= Qt::ItemIsEditable;
		flags |= QAbstractItemModel::flags(index);
	}

	return flags;
}

// drag
////////////////////////////////////////////////////////////////////////////////////////
const QString& treeMimeType() {
	static const QString type("text/xml");
	return type;
}

QStringList TreeModel::mimeTypes() const {
	return{ treeMimeType() };
}

Qt::DropActions dropActions()  {
	return Qt::CopyAction;
}

Qt::DropActions TreeModel::supportedDragActions() const {
	return dropActions();
}

QMimeData *TreeModel::mimeData(const QModelIndexList &indexes) const {
	if (indexes.size() != 1) {
		qWarning() << "no support drag for more than one item";
		return nullptr;
	}

	QMimeData* mime = new QMimeData();
	mime->setData(treeMimeType(), serialize(indexes.front()));
	return mime;
}

// drop
///////////////////////////////////////////////////////////////////////////////////
Qt::DropActions TreeModel::supportedDropActions() const {
	return dropActions();
}

bool TreeModel::dropMimeData_helper(
	const QMimeData *data,
	Qt::DropAction action,
	int row, int column,
	const QModelIndex &parent,
	bool checkOnly
) {
	if (!data->hasFormat(treeMimeType()))
		return false;

	if (action == Qt::IgnoreAction)
		return true;

	if (!(dropActions() & action)) {
		return false;
	}

	return deserialize(data->data(treeMimeType()), parent, checkOnly);
}

bool TreeModel::canDropMimeData(
	const QMimeData *data,
	Qt::DropAction action,
	int row, int column,
	const QModelIndex &parent
	) const {
	return const_cast<TreeModel*>(this)
		->dropMimeData_helper(data, action, row, column, parent, true);
}

bool TreeModel::dropMimeData(
	const QMimeData *data,
	Qt::DropAction action,
	int row, int column, const QModelIndex &parent
) {
	return dropMimeData_helper(data, action, row, column, parent, false);
}

// serialize
///////////////////////////////////////////////////////////////////////////////
QString cdata(const QString& data) {
	return (data.isEmpty()) ? QString() : QString("<![CDATA[%1]]>").arg(data);
}

void serialize(
	const TreeModel* model, 
	QTextStream& out, 
	const QModelIndex &root,
	bool serializeIndex = false
) {
	out << "<item>" << cdata(model->item(root)->data());

	if (serializeIndex) {
		QVector<QModelIndex> parents;
		if (root.isValid()) {
			auto parent = root.parent();
			while (parent.isValid()) {
				parents.push_back(parent);
				parent = parent.parent();
			}
		}
		std::reverse(parents.begin(), parents.end());
		for (auto parent : parents) {
			out << "<index>"
				<< cdata(model->item(parent)->data())
				<< "</index>";
		}
	}

	auto rowCount = model->rowCount(root);
	for (int row = 0; row < rowCount; ++row) {
		auto index = model->index(row, 0, root);
		if (index.isValid()) {
			serialize(model, out, index);
		}
	}

	out << "</item>";
}

QTextCodec* serializeCodec() {
	return QTextCodec::codecForName("UTF-8");
}

QByteArray TreeModel::serialize(const QModelIndex& root) const {
	QByteArray data;
	QTextStream out(&data);
	out.setCodec(serializeCodec());
	::serialize(this, out, root, true);
	out.flush();
	return data;
}

// deserialize
////////////////////////////////////////////////////////////////////////////////////////////////////
QString cdata(const QDomElement& item) {
	return item.firstChild().toCDATASection().data();
}

void deserialize(TreeModel* model, QDomElement item, const QModelIndex& parent = QModelIndex());

void deserialize(TreeModel* model, const QDomElement& item, int row, const QModelIndex& parent = QModelIndex()) {
	auto index = model->insert(cdata(item), row, parent);
	if (index.isValid()) {
		deserialize(model, item, index);
	}
}

void deserialize(TreeModel* model, QDomElement item, const QModelIndex& parent) {
	const QString itemTag(QStringLiteral("item"));
	int row = 0;
	for (item = item.firstChildElement(itemTag); !item.isNull();
		item = item.nextSiblingElement(itemTag), ++row) {
		deserialize(model, item, row, parent);
	}
}

QModelIndex index(const TreeModel* model, const QDomElement& item, QModelIndex index = QModelIndex()) {
	const QString indexTag(QStringLiteral("index"));
	for (auto indexItem = item.firstChildElement(indexTag); !indexItem.isNull();
		indexItem = indexItem.nextSiblingElement(indexTag)) {
		index = model->index(cdata(indexItem), index);
	}
	index = model->index(cdata(item), index);
	return index;
}


bool TreeModel::deserialize(const QByteArray& data, const QModelIndex& indexTo, bool checkOnly) {
	QDomDocument doc;

	QString errorMessage;
	int errorLine = -1, errorColumn = -1;
	if (!doc.setContent(serializeCodec()->toUnicode(data), &errorMessage, &errorLine, &errorColumn)) {
		qWarning()
			<< "Invalid xml. " << errorMessage
			<< ", Line:" << errorLine
			<< ", Column: " << errorColumn;
		return false;
	}

	auto item = doc.documentElement();

	if (item.isNull()) {
		qWarning() << "Invalid xml";
		return false;
	}

	auto indexFrom = ::index(this, item);

	bool isValidTo = indexTo.isValid();
	int row = 0;
	QModelIndex parent;
	if (isValidTo) {
		row = indexTo.row();
		parent = indexTo.parent();
	}

	bool isValidFrom = indexFrom.isValid();
	if (isValidFrom) {
		if (!isValidTo || parent != indexFrom.parent()) {
			return false;
		}
	}

	if (checkOnly) {
		return true;
	}

	if (isValidFrom) {
		int rowFrom = indexFrom.row();
		removeRow(rowFrom, parent);
	}

	if (isValidTo) {
		::deserialize(this, item, row, parent);
	}
	else {
		::deserialize(this, item);
	}
	
	return true;
}
