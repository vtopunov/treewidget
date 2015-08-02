#ifndef TREEITEM_H
#define TREEITEM_H

#include <QtCore>

class TreeItem
{
public:
	explicit TreeItem(TreeItem* parent = 0)
		: unique_(), children_(), data_(), parent_(parent) {}

    ~TreeItem() {
		qDeleteAll(children_);
	}

	TreeItem* child(int row) {
		return children_.value(row, nullptr);
	}

	int childCount() const {
		return children_.size();
	}

	const QString& data() const {
		return data_;
	}

	bool setData(const QString& data) {
		if (data == data_) {
			return false;
		}

		if (parent_) {
			if (parent_->unique_.contains(data)) {
				return false;
			}

			auto it = parent_->unique_.find(data_);
			if (it == unique_.end()) {
				return false;
			}
				
			auto index = *it;
			parent_->unique_.erase(it);
			parent_->unique_.insert(data, index);
		}
		
		data_ = data;

		return true;
	}

	void removeChildren(int position, int count) {	
		for(int row = 0; row < count; ++row) {
			QScopedPointer<TreeItem> item(children_.takeAt(position));
			unique_.remove(item->data());
		}
	}

	TreeItem* insertChildren(const QString& data, int pos = -1) {
		auto child = this->child(unique_.value(data, -1));
		if (child) {
			return child;
		}
		
		if (pos < 0 || pos >= children_.size()) {
			pos = children_.size();
		}

		unique_.insert(data, pos);

		QScopedPointer<TreeItem> item(new TreeItem(this));
		item->data_ = data;
		children_.insert(pos, item.data());
		return item.take();
	}

	TreeItem* parent() const {
		return parent_;
	}

	int row() const {
		return findSibling(data(), 0);
	}

	bool isEmptyChild() const {
		return unique_.contains({});
	}

	bool isEmpty() const {
		return data_.isEmpty();
	}

	int findChild(const QString& data, int defaultIndex = -1) const {
		return unique_.value(data, defaultIndex);
	}

	int findSibling(const QString& data, int defaultIndex = -1) const {
		return (parent_) ? parent_->findChild(data, defaultIndex) : defaultIndex;
	}

private:
	QHash<QString, int> unique_;
	QList<TreeItem*> children_;
    QString data_;
    TreeItem* parent_;
};

#endif
