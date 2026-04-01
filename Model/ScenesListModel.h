#ifndef SCENESLISTMODEL_H
#define SCENESLISTMODEL_H

#include "QAbstractListModelPub.h"
#include "SceneInfoManager.h"
#include "PaginatedList.h"
#include <utility>
#include <QSet>
#include <QFileInfo>

extern template class PaginatedList<SceneInfo>;
using PaginatedSceneList = PaginatedList<SceneInfo>;

class ScenesListModel : public QAbstractListModelPub {
  Q_OBJECT
 public:
  explicit ScenesListModel(const QString& listViewName, QObject* object = nullptr);
  bool isSortProxyInited() const { return m_bSortProxyInited; }
  bool initSortSetting(SceneInfo::Role newSortDimension, bool bResultReverse);
  bool setSortDimension(SceneInfo::Role newSortDimension);
  bool setSortResultReverse(bool bResultReverse);

  bool setRootPath(const QString& rootPath, const bool bForce = false);
  inline QString rootPath() const { return mRootPath; }

  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return mPagedData.GetLocalEleCnt(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  QFileInfo fileInfo(const QModelIndex& index) const;
  QString filePath(const QModelIndex& index) const;
  QString fileName(const QModelIndex& index) const;
  int GetRate(const QModelIndex& index) const;
  QString baseName(const QModelIndex& index) const;
  QString absolutePath(const QModelIndex& index) const;
  QStringList GetImgs(const QModelIndex& index) const;
  QStringList GetVids(const QModelIndex& index) const;
  QString GetJson(const QModelIndex& index) const;
  QString GetScn(const QModelIndex& index) const;

  QStringList rel2fileNames(const QModelIndexList& indexes) const;

  bool isLocalIndexValid(const QModelIndex& localIndex, int& localInd) const { return mPagedData.isLocalIndexValid(localIndex, localInd); }
  SceneInfoList::const_iterator constBeginCurPage() const { return mPagedData.constBeginCurPage(); }

 signals:
  void pagesCountChanged(int newPagesCount);

 public slots:
  bool onScenesCountsPerPageChanged(int scenesCntInAPage);
  bool onPageIndexChanged(int newPageIndex);
  int AfterJsonFilesNameRenamed(const QModelIndexList& indexes);
  void EmitPagesCountChanged(int newPagesCount) { emit pagesCountChanged(newPagesCount); }
  bool onDisableImageDecorationChanged(bool bDisabled);

 private:
  bool ModifySceneInfoRateValue(const QModelIndex& index, int newRate);

  QString mPattern;
  QString mRootPath;
  PaginatedSceneList mPagedData;
  bool m_bSortProxyInited{false};
  bool m_bDisableImage{false};
};
#endif  // SCENESLISTMODEL_H
