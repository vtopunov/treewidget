#ifndef BUTTONSDELEGATE_H
#define BUTTONSDELEGATE_H

#include <QItemDelegate>
#include <QVector>

class ButtonsDelegate : public QItemDelegate
{
	Q_OBJECT


public:
	enum ButtonsType {
		ADD_ITEM_BUTTON,
		ADD_CHILD_ITEM_BUTTON,
		REMOVE_ITEM,
		BUTTON_TYPE_COUNT
	};

public:
	ButtonsDelegate(const QSize& size, QObject *parent = 0);

signals:
	void buttonClicked(ButtonsType type, const QModelIndex& index);
	void addClicked(const QModelIndex& index);
	void addChildClicked(const QModelIndex& index);
	void removeClicked(const QModelIndex& index);

public:
	void paint(
		QPainter *painter, 
		const QStyleOptionViewItem &option, 
		const QModelIndex &index
	) const Q_DECL_OVERRIDE;

	QSize sizeHint(
		const QStyleOptionViewItem &option, 
		const QModelIndex &index
	) const Q_DECL_OVERRIDE;

	QWidget *createEditor(
		QWidget *parent,
		const QStyleOptionViewItem &option,
		const QModelIndex &index
	) const Q_DECL_OVERRIDE;;

protected:
	bool editorEvent(
		QEvent *qevent,
		QAbstractItemModel *model,
		const QStyleOptionViewItem &option,
		const QModelIndex &index
	) Q_DECL_OVERRIDE;

protected:
	QVector<QPixmap> icons_;
	QLineEdit* editor_;
	QString placeholderText_;
};

#endif // BUTTONSDELEGATE_H
