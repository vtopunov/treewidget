#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "treeitem.h"


inline TreeItem* item(const QModelIndex& index, TreeItem* defValue = 0) {
	return (index.isValid())
		? static_cast<TreeItem*>(index.internalPointer())
		: defValue;
}

const QString& treeMimeType();

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TreeModel(QObject* parent = 0);
    ~TreeModel();

public:
	QModelIndex addItem(const QString& data, const QModelIndex& parent = QModelIndex());

    QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;

    QModelIndex index(int row, int column,
		const QModelIndex& parent = QModelIndex() ) const Q_DECL_OVERRIDE;

    QModelIndex parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
	
    int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
	
	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

	bool setData(
		const QModelIndex &index, 
		const QVariant &value,
		int role = Qt::EditRole
	) Q_DECL_OVERRIDE;

	bool insertRows(int position, int rows,
		const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;

	bool removeRows(int position, int rows,
		const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;

	QStringList mimeTypes() const Q_DECL_OVERRIDE;

	Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;

	QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;

	bool dropMimeData(
		const QMimeData *data, 
		Qt::DropAction action,
		int row, int column, const QModelIndex &parent
	) Q_DECL_OVERRIDE;

private:
	TreeItem* root_;
};

#endif // TREEMODEL_H
