#ifndef AIMEDIADUPTABLEVIEW_H
#define AIMEDIADUPTABLEVIEW_H

#include "CustomTableView.h"
#include "AiMediaTablesModel.h"
class QSortFilterProxyModel;

class AiMediaDupTableView : public CustomTableView {
 public:
  friend class DuplicateVideosFinder;
  AiMediaDupTableView(QWidget* parent = nullptr);
  void LoadAiMediaTableNames();
  bool onScanAPath(const QString& testUserSpecifiedPath = ""); // para testUserSpecifiedPath for test user only
  QStringList GetSelectedAiTables() const;
 private:
  QModelIndexList Proxy2Source(const QModelIndexList& proInds) const;
  void subscribe();
  AiMediaTablesModel* m_aiMediaTblModel{nullptr};
  QMenu* m_aiMediaDupMenu{nullptr};
  QSortFilterProxyModel* m_sortProxy{nullptr};
};

#endif // AIMEDIADUPTABLEVIEW_H
