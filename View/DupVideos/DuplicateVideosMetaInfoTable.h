#ifndef DUPLICATEVIDEOSMETAINFOTABLE_H
#define DUPLICATEVIDEOSMETAINFOTABLE_H

#include "CustomTableView.h"
#include "DuplicateVideosMetaInfoModel.h"
#include "DupVidsManager.h"

class QSortFilterProxyModel;

class DuplicateVideosMetaInfoTable : public CustomTableView {
  Q_OBJECT
 public:
  friend class DuplicateVideosFinder;
  explicit DuplicateVideosMetaInfoTable(QWidget* parent = nullptr);
  void LoadAiMediaTableNames();
  bool onScanAPath(const QString& specifiedPath = "");
  QStringList GetSelectedAiTables() const;
  QString GetCurrentDupVideoMetaInfo() const;

 signals:
  void analyzeTablesFinished(const DupVidMetaInfoList& videosListNeedAnalyse);  // callee should emit leftGroupsTableSelectionChanged(-1);

 protected:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dropEvent(QDropEvent* event) override;

 private:
  QModelIndexList Proxy2Source(const QModelIndexList& proInds) const;
  int startAnalyzeNewTables(const QStringList& tablesNeedAnalyze);
  int onAnalyzeTheseSelectedTables();
  int onClearAnalyzeList();
  bool onDropSelectedTables();
  bool onAuditSelectedTables();
  bool onForceReloadTables();
  bool onOpenTableAssociatedPath(const QModelIndex& ind);
  void subscribe();

  DupVidMetaInfoList mVideosListNeedAnalyse;

  DupVidsManager mDupVidMngr;
  DuplicateVideosMetaInfoModel* m_aiMediaTblModel{nullptr};
  QMenu* m_aiMediaDupMenu{nullptr};
  QSortFilterProxyModel* m_sortProxy{nullptr};
};

#endif  // DUPLICATEVIDEOSMETAINFOTABLE_H
