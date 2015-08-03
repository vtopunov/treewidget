#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QtWidgets>

#include "treemodel.h"
#include "sortfilterproxymodel.h"
#include "buttonsdelegate.h"

class TreeWidget : public QTreeView
{
	Q_OBJECT

public:
	TreeWidget(QWidget *parent = 0);
	~TreeWidget();

public slots:
	void closeEditor();
	void insertRow(int row, const QModelIndex& parent = QModelIndex());
	void removeRow(int row, const QModelIndex& parent = QModelIndex());
	void search(const QString& searchText);
	QByteArray serialize() const;
	void deserialize(const QByteArray& data);

private:
	TreeModel* sourceModel_;
	SortFilterProxyModel* model_;
	ButtonsDelegate* itemDelegate_;
};

#endif // TREEWIDGET_H
