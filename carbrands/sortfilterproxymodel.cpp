#include "sortfilterproxymodel.h"
#include <qdebug.h>

SortFilterProxyModel::SortFilterProxyModel(QObject* parent) :
	QSortFilterProxyModel(parent),
	cache_(),
	regExpCache_(),
	sourceModelCache_(nullptr)
{}

bool acceptData(const QRegExp& regExp, const QVariant& data) {
	auto string = data.toString().trimmed();
	return string.isEmpty() || regExp.isEmpty() || string.contains(regExp);
}

void SortFilterProxyModel::clearCache() const {
	cache_.clear();
}

bool SortFilterProxyModel::setData(const QModelIndex &index, const QVariant& value, int role) {
	auto filterRole = this->filterRole();
	if (role == filterRole
		|| (filterRole == Qt::DisplayRole && role == Qt::EditRole)) {
		if (!acceptData(filterRegExp(), value)) {
			return false;
		}
	}

	return QSortFilterProxyModel::setData(index, value);
}

void SortFilterProxyModel::updateCache() const {
	bool recache = false;

	auto sourceModel = this->sourceModel();
	if (sourceModelCache_ != sourceModel) {
		if (sourceModelCache_) {
			disconnect(sourceModelCache_, 0, this, 0);
		}

		if (sourceModel) {
			connect(sourceModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(clearCache()));

			connect(sourceModel, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(clearCache()));

			connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(clearCache()));
		}

		sourceModelCache_ = sourceModel;
		recache = true;
	}

	auto regExp = filterRegExp();
	if (regExpCache_ != regExp) {
		regExpCache_ = regExp;
		recache = true;
	}

	if (recache) {
		clearCache();
	}
}

SortFilterProxyModel::CacheItem& 
	SortFilterProxyModel::cachedAccept(
		const QModelIndex& sourceIndex,
		bool* inCachePtr
) const {
	auto it = cache_.find(sourceIndex);
	
	bool inCache = it != cache_.end();
	if (inCachePtr) {
		*inCachePtr = inCache;
	}

	if (inCache) {
		return it.value();
	}

	auto filterData = sourceModel()->data(sourceIndex, this->filterRole());

	auto acceptData = ::acceptData(filterRegExp(), filterData);

	return cache_.insert(sourceIndex, { acceptData, false }).value();
}

bool SortFilterProxyModel::filterAcceptsRow(
	int sourceRow, 
	const QModelIndex &sourceParent
) const {
	updateCache();

	auto sourceModel = this->sourceModel();
	if (!sourceModel) {
		qWarning() << "source model is null";
		return true;
	}
	
	auto makeIndex = [sourceModel, this](int row, const QModelIndex &parent) {
		return sourceModel->index(row, this->filterKeyColumn(), parent);
	};

	auto sourceIndex = makeIndex(sourceRow, sourceParent);
	if (!sourceIndex.isValid()) {
		qWarning() << "invalid source model index";
		return true;
	}

	bool inCache;
	auto& cacheItem = cachedAccept(sourceIndex, &inCache);

	if (inCache || cacheItem.isAccept) {
		return cacheItem.isAccept;
	}
	
	if (sourceIndex.isValid()){
		auto rowCount = sourceModel->rowCount(sourceIndex);
		for (int i = 0; i < rowCount; ++i){
			if (cachedAccept(makeIndex(i, sourceIndex)).isAccept) {
				return (cacheItem.isExpand = true);
			}
		}
	}

	auto parent = sourceParent;
	while (parent.isValid()) {
		if (cachedAccept(parent).isAccept) {
			return true;
		}
		parent = parent.parent();
	}

	return false;
}

QSet<QModelIndex> SortFilterProxyModel::indexesExpand() const {
	QSet<QModelIndex> expandItems;
	for (auto it = cache_.begin(); it != cache_.end(); ++it) {
		if (it.value().isExpand) {
			auto index = it.key();
			if (index.isValid()) {
				expandItems.insert(index);
			}
		}
	}

	QSet<QModelIndex> proxyExpandItems;
	for (auto& expandItem : expandItems) {
		auto index = mapFromSource(expandItem);
		if (index.isValid()) {
			proxyExpandItems.insert(index);
		}
	}

	return proxyExpandItems;
}
