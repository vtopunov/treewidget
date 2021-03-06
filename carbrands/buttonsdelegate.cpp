#include <QtGui>
#include <QtWidgets>

#include "buttonsdelegate.h"

enum { WIDTH_MARGIN = 2 };

class ButtonIterate {
public:
	ButtonIterate(const QVector<QPixmap>& icons, bool isHideAddChild) :
		icons_(icons),
		isHideAddChild_(isHideAddChild),
		picon_(nullptr),
		buttonIndex_(icons_.size())
	{
	}

	const QPixmap& icon() const { return *picon_; }

	bool isHide() const { 
		return isHideAddChild_ && type() == ButtonsDelegate::ADD_CHILD_ITEM_BUTTON;
	}

	bool next() {
		return (--buttonIndex_ >= 0) && (picon_ = &icons_[buttonIndex_], true);
	}

	int width() const { return icon().width() + WIDTH_MARGIN; }

	int height() const { return icon().height(); }

	ButtonsDelegate::ButtonsType type() const {
		return static_cast<ButtonsDelegate::ButtonsType>(buttonIndex_);
	}

private:
	const QVector<QPixmap>& icons_;
	const bool isHideAddChild_;
	const QPixmap* picon_;
	int buttonIndex_;
};

bool isHideAddChild(const QModelIndex& index) {
	return index.parent().isValid();
}

class ButtonPositionIterate : public ButtonIterate {
public:
	ButtonPositionIterate(
		const QVector<QPixmap>& icons, 
		const QStyleOptionViewItem &option, 
		bool isHideAddChild
	) : ButtonIterate(icons, isHideAddChild), 
		pos_(option.rect.right(), option.rect.center().y()) {}

	QPoint nextPos() { 
		return QPoint(
			pos_.rx() -= width(), 
			pos_.y() - height()/2
		);
	}

private:
	QPoint pos_;
};

ButtonsDelegate::ButtonsDelegate(const QSize& size, QObject *parent)
	: QItemDelegate(parent),
	icons_(BUTTON_TYPE_COUNT),
	placeholderText_(tr("Enter text"))
{
	auto buttonInit = [this, &size](
		ButtonsType type, 
		void (ButtonsDelegate::*signal) (const QModelIndex&), 
		const QString& path
	) {
		this->icons_[type] = QIcon(path).pixmap(size);
		QObject::connect(this, &ButtonsDelegate::buttonClicked, 
			[this, type, signal](ButtonsType clickedType, const QModelIndex& index) { 
				if (clickedType == type) {
					emit (this->*signal)(index);
				}
			}
		);
	};

	buttonInit(ADD_ITEM_BUTTON,			&ButtonsDelegate::addClicked,		":/add.png"		);
	buttonInit(ADD_CHILD_ITEM_BUTTON,	&ButtonsDelegate::addChildClicked,	":/branch.png"	);
	buttonInit(REMOVE_ITEM,				&ButtonsDelegate::removeClicked,	":/delete.png"	);
}

void ButtonsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	QItemDelegate::paint(painter, option, index);
	
	if (index.data(Qt::DisplayRole).toString().isEmpty()) {
		auto oldPen = painter->pen();
		painter->setPen(QColor(0, 0, 0, 64));
		painter->drawText(
			option.rect,
			placeholderText_,
			QTextOption(Qt::AlignVCenter)
		);
		painter->setPen(oldPen);
	}

	if (option.state & QStyle::State_MouseOver) {
		ButtonPositionIterate button(icons_, option, index.parent().isValid());
		while (button.next()) {
			if (!button.isHide()) {
				painter->drawPixmap(button.nextPos(), button.icon());
			}
		}
	}
}

QSize buttonsSize(const QVector<QPixmap>& icons, bool isHideAddChild, QSize size = QSize()) {
	ButtonIterate button(icons, isHideAddChild);
	while (button.next()) {
		if (!button.isHide()) {
			size.rwidth() += button.width();
			size.setHeight(qMax(size.height(), button.height()));
		}
	}
	return size;
}

QSize ButtonsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return buttonsSize(
		icons_, 
		isHideAddChild(index), 
		QItemDelegate::sizeHint(option, index)
	);
}

QWidget *ButtonsDelegate::createEditor(
	QWidget *parent,
	const QStyleOptionViewItem &option,
	const QModelIndex &index
) const {
	auto editor = QItemDelegate::createEditor(parent, option, index);
	auto lineEdit = qobject_cast<QLineEdit*>(editor);
	if (lineEdit) {
		lineEdit->setPlaceholderText(placeholderText_);
	}
	return lineEdit;
}

void ButtonsDelegate::updateEditorGeometry(
	QWidget *editor,
	const QStyleOptionViewItem &option,
	const QModelIndex &index
) const {
	auto lineEdit = qobject_cast<QLineEdit*>(editor);
	if (lineEdit) {
		auto rect = option.rect;
		rect.setWidth(option.rect.width()
			- buttonsSize(icons_, false).width()
		);
		lineEdit->setGeometry(rect);
	}
}

bool ButtonsDelegate::editorEvent(
	QEvent *qevent,
	QAbstractItemModel*,
	const QStyleOptionViewItem &option,
	const QModelIndex &index
) {
	auto type = qevent->type();
	bool isRelease = type == QEvent::MouseButtonRelease;

	if (isRelease ||
		type == QEvent::MouseButtonDblClick || 
		type == QEvent::MouseButtonPress ||
		type == QEvent::MouseMove ) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(qevent);
		
		ButtonPositionIterate button(icons_, option, isHideAddChild(index));
		while (button.next()) {
			if (!button.isHide()) {
				auto pos = button.nextPos().x();
				auto endPos = pos + button.width();
				auto mousePos = mouseEvent->pos().x();
				if (mousePos >= pos && mousePos < endPos) {
					if (isRelease) {
						emit buttonClicked(button.type(), index);
					}
					return true;
				}
			}
		}
	}

	return false;
}



