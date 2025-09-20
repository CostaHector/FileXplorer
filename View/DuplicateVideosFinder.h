#ifndef DUPLICATEVIDEOSFINDER_H
#define DUPLICATEVIDEOSFINDER_H

#include "CustomTableView.h"
#include "AiMediaDupTableView.h"

#include "DuplicateVideoModel.h"

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QSplitter>
#include <QToolBar>

class DuplicateVideosFinder;
class QSortFilterProxyModel;

class LeftDuplicateList : public CustomTableView {
 public:
  friend class DuplicateVideosFinder;
  explicit LeftDuplicateList(QWidget* parent = nullptr);

 private:
  void subscribe();
  VidInfoModel* m_dupListModel{nullptr};
  QSortFilterProxyModel* m_sortProxy{nullptr};
};
// -------------------------------------------------------------------------------------------------
class RightDuplicateDetails : public CustomTableView {
 public:
  friend class DuplicateVideosFinder;
  explicit RightDuplicateDetails(QWidget* parent = nullptr);
  void on_effectiveNameCopiedForEverything(const QModelIndex& ind) const;
  void on_cellDoubleClicked(const QModelIndex& ind) const;
  void setSharedMember(CLASSIFIED_SORT_LIST_2D* pClassifiedSort, DIFFER_BY_TYPE* pCurDifferType);
  void onRecycleSelection();

 private:
  void subscribe();
  DuplicateDetailsModel* m_detailsModel{nullptr};
  QSortFilterProxyModel* m_sortProxy{nullptr};
};
// -------------------------------------------------------------------------------------------------
class DuplicateVideosFinder : public QMainWindow {
 public:
  explicit DuplicateVideosFinder(QWidget* parent = nullptr);
  bool TablesGroupChangedTo(const QStringList& tbls);
  
  void onAnalyseAiMediaTableChanged();
  void onCancelAnalyse();
  void onDifferTypeChanged(QAction* newDifferAct);

  void onChangeSizeDeviation();
  void onChangeDurationDeviation();
  void on_selectionChanged(const QModelIndex &current, const QModelIndex &/*previous*/);

  void UpdateAiMediaTableNames();

  void UpdateWindowsTitle(int tablesInAnalyseCnt = 0);

  void updateWindowsSize();

  void showEvent(QShowEvent *event) override;
  void closeEvent(QCloseEvent* event) override;

  void keyPressEvent(QKeyEvent* e) override;

  bool operator()(const QString& /**/) {return true;}
 private:
  void subscribe();

  QToolBar* m_tb{nullptr};
  LeftDuplicateList* m_dupList{nullptr};
  RightDuplicateDetails* m_details{nullptr};

  AiMediaDupTableView* m_aiTables{nullptr};

  QSplitter* m_detail_left_right{nullptr};
  QSplitter* m_tbl_detail_ver{nullptr};

  static const QString DUPLICATE_FINDER_TITLE_TEMPLATE;
};

#endif  // DUPLICATEVIDEOSFINDER_H
