#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class SortFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	struct CacheItem {
		bool isAccept;
		bool isExpand;
	};

	typedef QHash<QModelIndex, CacheItem> Cache;

public:
	SortFilterProxyModel(QObject* source);

public:
	bool setData(
		const QModelIndex &index,
		const QVariant &value,
		int role = Qt::EditRole
	) Q_DECL_OVERRIDE;
	
	QSet<QModelIndex> indexesExpand() const;

protected:
	virtual bool filterAcceptsRow(
		int sourceRow, 
		const QModelIndex &sourceParent
	) const Q_DECL_OVERRIDE;

private slots:
	void clearCache() const;

private:
	void updateCache() const;
	CacheItem& cachedAccept(const QModelIndex& sourceIndex, bool* inCache = nullptr) const;

private:
	mutable Cache cache_;
	mutable QRegExp regExpCache_;
	mutable QAbstractItemModel* sourceModelCache_;
};

#endif // SORTFILTERPROXYMODEL_H
