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
		for (int i = position + count; i < children_.size(); ++i) {
			unique_[children_[i]->data()] = i - count;
		}

		for(int row = 0; row < count; ++row) {
			QScopedPointer<TreeItem> item(children_.takeAt(position));
			unique_.remove(item->data());
		}
	}

	bool insertChildren(const QString& data, int pos = -1) {
		if (unique_.contains(data)) {
			return false;
		}
		
		if (pos < 0 || pos >= children_.size()) {
			pos = children_.size();
		}

		unique_[data] = pos;
		for (int i = pos; i < children_.size(); ++i) {
			unique_[children_[i]->data_] = i + 1;
		}

		QScopedPointer<TreeItem> item(new TreeItem(this));
		item->data_ = data;
		children_.insert(pos, item.data());
		item.take();

		return true;
	}

	TreeItem* parent() const {
		return parent_;
	}

	int row() const {
		return (parent_) ? parent_->index(data_) : 0;
	}

	bool isEmpty() const {
		return data_.isEmpty();
	}

	int index(const QString& data, int defaultIndex = -1) const {
		return unique_.value(data, defaultIndex);
	}

private:
	QHash<QString, int> unique_;
	QList<TreeItem*> children_;
    QString data_;
    TreeItem* parent_;
};

#endif
