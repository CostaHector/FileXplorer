#ifndef MULTIPARVIEW_H
#define MULTIPARVIEW_H

#include "CustomTableView.h"
#include "MultiParModel.h"
#include <QSortFilterProxyModel>

class MultiParView : public CustomTableView {
  Q_OBJECT
public:
  explicit MultiParView(ParVerifyInfomationList&& resultList, const QString& instName, QWidget* parent = nullptr);
  void setFilter(const QString& filter);

signals:
  void showCliOutputReq(const QByteArray& cliOutput);

private:
  void subscribe();
  void onSelectionChange(const QModelIndex& proIndex);
  int onRepairBrokenFile();
  int onSyncPar2FileName();
  int onFixMisnamedFile();
  int onProcessOldNewPar2NamesThenReverify();

  QAction* m_repairBrokenFile{nullptr};
  QAction* m_syncBuiltInSrcFileListInPar2{nullptr};
  QAction* m_repairMissnamedFile{nullptr};
  QAction* m_processOldNewPar2NameThenReverify{nullptr};

  MultiParModel* mMultiParModel{nullptr};
  QSortFilterProxyModel* mSortFilterProxy{nullptr};
};

#endif // MULTIPARVIEW_H
